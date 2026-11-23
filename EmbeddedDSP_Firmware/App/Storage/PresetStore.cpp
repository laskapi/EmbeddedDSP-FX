#include "Storage/PresetStore.h"
#include "Protocol/Checksums.h"

#include <cstring>

#ifndef FIRMWARE_UNIT_TESTS
#include "stm32f4xx_hal.h"

// Linker symbols from STM32F411XX_FLASH.ld (must be global, not in a C++ namespace).
extern "C" {
extern const uint8_t _flash_user_start[];
extern const uint8_t _flash_user_end[];
}
#endif

namespace App::Storage {

namespace {

#ifdef FIRMWARE_UNIT_TESTS
alignas(8) uint8_t presetRam[64 * 1024];
#endif

} // namespace

uint16_t PresetRecord::computeCrc(const PresetRecord& rec) noexcept {
    return Protocol::Checksums::crc16(reinterpret_cast<const uint8_t*>(&rec),
                                      offsetof(PresetRecord, crc));
}

PresetRecord PresetRecord::make(uint8_t presetId,
                                uint32_t sequence,
                                const Dsp::ChainSettings& settings) noexcept {
    PresetRecord rec{};
    rec.presetId = presetId;
    rec.sequence = sequence;
    rec.settings = settings;
    rec.crc = computeCrc(rec);
    return rec;
}

bool PresetStore::isValidRecord(const PresetRecord& rec) noexcept {
    if (rec.magic != PresetRecord::kMagic || rec.version != PresetRecord::kVersion) {
        return false;
    }
    if (rec.presetId >= kPresetCount) {
        return false;
    }
    if (rec.settings.magic != Dsp::ChainSettings::kMagic
        || rec.settings.version != Dsp::ChainSettings::kVersion) {
        return false;
    }
    return PresetRecord::computeCrc(rec) == rec.crc;
}

const uint8_t* PresetStore::storageAddress() const noexcept {
#ifndef FIRMWARE_UNIT_TESTS
    return _flash_user_start;
#else
    return presetRam;
#endif
}

std::size_t PresetStore::storageSize() const noexcept {
#ifndef FIRMWARE_UNIT_TESTS
    return static_cast<std::size_t>(_flash_user_end - _flash_user_start);
#else
    return sizeof(presetRam);
#endif
}

std::size_t PresetStore::maxRecords() const noexcept {
    return storageSize() / kPresetRecordSize;
}

void PresetStore::setActivePreset(uint8_t presetId) noexcept {
    m_activePreset = presetId;
}

void PresetStore::rebuildIndex() noexcept {
    for (auto& off : m_latestOffset) {
        off = -1;
    }
    m_nextSequence = 1;
    m_writeOffset = 0;

    const uint8_t* base = storageAddress();
    const std::size_t capacity = maxRecords();

    for (std::size_t i = 0; i < capacity; ++i) {
        PresetRecord rec{};
        std::memcpy(&rec, base + i * kPresetRecordSize, sizeof(rec));
        // Erased flash word — end of contiguous append log.
        if (rec.magic == 0xFFFFFFFFu) {
            m_writeOffset = i * kPresetRecordSize;
            break;
        }
        if (!isValidRecord(rec)) {
            continue;
        }
        m_latestOffset[rec.presetId] = static_cast<int32_t>(i * kPresetRecordSize);
        m_nextSequence = rec.sequence + 1;
        m_writeOffset = (i + 1) * kPresetRecordSize;
    }
}

void PresetStore::init() noexcept {
#ifdef FIRMWARE_UNIT_TESTS
    std::memset(presetRam, 0xFF, sizeof(presetRam));
#endif
    rebuildIndex();
    m_activePreset = 0;
}

bool PresetStore::tryReadLatest(uint8_t presetId, Dsp::ChainSettings& out) const noexcept {
    if (presetId >= kPresetCount || m_latestOffset[presetId] < 0) {
        return false;
    }
    PresetRecord rec{};
    std::memcpy(&rec, storageAddress() + m_latestOffset[presetId], sizeof(rec));
    if (!isValidRecord(rec)) {
        return false;
    }
    out = rec.settings;
    return true;
}

bool PresetStore::load(uint8_t presetId, Dsp::ChainSettings& out) const noexcept {
    if (presetId >= kPresetCount) {
        return false;
    }
    if (!tryReadLatest(presetId, out)) {
        out = Dsp::ChainSettings::makeEmpty();
    }
    return true;
}

bool PresetStore::eraseStorage() noexcept {
#ifndef FIRMWARE_UNIT_TESTS
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase{};
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = FLASH_SECTOR_7;
    erase.NbSectors = 1;
    uint32_t sectorError = 0;
    const HAL_StatusTypeDef st = HAL_FLASHEx_Erase(&erase, &sectorError);
    HAL_FLASH_Lock();
    return st == HAL_OK;
#else
    std::memset(presetRam, 0xFF, sizeof(presetRam));
    return true;
#endif
}

bool PresetStore::writeStorage(std::size_t offset, const void* data, std::size_t len) noexcept {
    if (offset + len > storageSize()) {
        return false;
    }
#ifndef FIRMWARE_UNIT_TESTS
    const auto base = reinterpret_cast<uintptr_t>(_flash_user_start);
    const auto* words = static_cast<const uint32_t*>(data);
    HAL_FLASH_Unlock();
    for (std::size_t i = 0; i < len / 4; ++i) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, base + offset + i * 4, words[i])
            != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }
    HAL_FLASH_Lock();
    return true;
#else
    std::memcpy(presetRam + offset, data, len);
    return true;
#endif
}

bool PresetStore::appendRecord(const PresetRecord& rec) noexcept {
    if (m_writeOffset + kPresetRecordSize > storageSize()) {
        return false;
    }
    if (!writeStorage(m_writeOffset, &rec, kPresetRecordBytes)) {
        return false;
    }
    m_latestOffset[rec.presetId] = static_cast<int32_t>(m_writeOffset);
    m_writeOffset += kPresetRecordSize;
    return true;
}

bool PresetStore::compactStorage() noexcept {
    Dsp::ChainSettings keep[kPresetCount]{};
    bool have[kPresetCount]{};
    for (uint8_t id = 0; id < kPresetCount; ++id) {
        // Only rewrite banks that actually have a stored record (not makeEmpty).
        have[id] = tryReadLatest(id, keep[id]);
    }

    if (!eraseStorage()) {
        return false;
    }
    m_writeOffset = 0;
    m_nextSequence = 1;
    for (auto& off : m_latestOffset) {
        off = -1;
    }

    for (uint8_t id = 0; id < kPresetCount; ++id) {
        if (!have[id]) {
            continue;
        }
        if (!appendRecord(PresetRecord::make(id, m_nextSequence++, keep[id]))) {
            return false;
        }
    }
    return true;
}

bool PresetStore::save(uint8_t presetId, const Dsp::ChainSettings& in) noexcept {
    if (presetId >= kPresetCount) {
        return false;
    }
    if (in.magic != Dsp::ChainSettings::kMagic || in.version != Dsp::ChainSettings::kVersion) {
        return false;
    }

    if (m_writeOffset + kPresetRecordSize > storageSize()) {
        if (!compactStorage()) {
            return false;
        }
    }

    const PresetRecord rec = PresetRecord::make(presetId, m_nextSequence++, in);
    if (!appendRecord(rec)) {
        return false;
    }
    m_activePreset = presetId;
    return true;
}

} // namespace App::Storage
