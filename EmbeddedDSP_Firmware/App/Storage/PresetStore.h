#ifndef EMBEDDEDDSP_FIRMWARE_PRESET_STORE_H
#define EMBEDDEDDSP_FIRMWARE_PRESET_STORE_H

#include "DSP/ChainSettings.h"
#include "Protocol/ProtocolCommon.h"
#include <cstdint>
#include <cstddef>

namespace App::Storage {

/// @brief One append-log entry in FLASH_USER (or RAM under unit tests).
#pragma pack(push, 1)
struct alignas(8) PresetRecord {
    static constexpr uint32_t kMagic = 0x50525354; // "PRST"
    static constexpr uint16_t kVersion = 1;

    uint32_t magic{kMagic};
    uint16_t version{kVersion};
    uint8_t presetId{0};
    uint8_t reserved{0};
    uint32_t sequence{0};
    Dsp::ChainSettings settings{};
    uint16_t crc{0};
    uint16_t reserved2{0};

    /// @brief CRC over packed fields before crc/reserved2.
    [[nodiscard]] static uint16_t computeCrc(const PresetRecord& rec) noexcept;

    /// @brief Builds a new log record (magic/version/crc set).
    [[nodiscard]] static PresetRecord make(uint8_t presetId,
                                           uint32_t sequence,
                                           const Dsp::ChainSettings& settings) noexcept;
};
#pragma pack(pop)

/// @brief Packed payload size (excludes trailing alignas padding).
inline constexpr std::size_t kPresetRecordBytes =
    offsetof(PresetRecord, reserved2) + sizeof(PresetRecord::reserved2);

/// @brief Full object / log slot size (includes alignas padding).
inline constexpr std::size_t kPresetRecordSize = sizeof(PresetRecord);

static_assert(kPresetRecordSize % 8u == 0u,
              "PresetRecord size must be 8-byte aligned for flash slots");

/// @brief Append-only user presets in reserved Flash sector (10 slots).
class PresetStore {
public:
    static constexpr uint8_t kPresetCount = Protocol::kUserPresetCount;

    /// @brief Scan media and rebuild latest record per preset id.
    void init() noexcept;

    /// @brief Loads bank @p presetId; missing/corrupt → makeEmpty (always succeeds for valid id).
    [[nodiscard]] bool load(uint8_t presetId, Dsp::ChainSettings& out) const noexcept;
    [[nodiscard]] bool save(uint8_t presetId, const Dsp::ChainSettings& in) noexcept;

    [[nodiscard]] uint8_t activePreset() const noexcept { return m_activePreset; }
    void setActivePreset(uint8_t presetId) noexcept;

private:
    [[nodiscard]] static bool isValidRecord(const PresetRecord& rec) noexcept;
    [[nodiscard]] const uint8_t* storageAddress() const noexcept;
    [[nodiscard]] std::size_t storageSize() const noexcept;
    [[nodiscard]] std::size_t maxRecords() const noexcept;

    /// @brief True only when a valid flash/RAM record exists for @p presetId.
    [[nodiscard]] bool tryReadLatest(uint8_t presetId, Dsp::ChainSettings& out) const noexcept;

    void rebuildIndex() noexcept;
    [[nodiscard]] bool appendRecord(const PresetRecord& rec) noexcept;
    [[nodiscard]] bool compactStorage() noexcept;
    [[nodiscard]] bool eraseStorage() noexcept;
    [[nodiscard]] bool writeStorage(std::size_t offset, const void* data, std::size_t len) noexcept;

    int32_t m_latestOffset[kPresetCount]{};
    uint32_t m_nextSequence{1};
    std::size_t m_writeOffset{0};
    uint8_t m_activePreset{0};
};

} // namespace App::Storage

#endif // EMBEDDEDDSP_FIRMWARE_PRESET_STORE_H
