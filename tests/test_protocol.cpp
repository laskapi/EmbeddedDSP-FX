#include <gtest/gtest.h>
#include "Protocol/ControlPacket.h"
#include "Comm/UsbRx.h"
#include "Comm/UsbTx.h"
#include "Comm/CommandDispatcher.h"
#include "DSP/MasterVolume.h"
#include "Protocol/ProtocolCommon.h"
#include "DSP/FxChain.h"
#include "DSP/OverdriveEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/EmptyEffect.h"

#include <cstring>
#include <string>
#include <vector>

using namespace Protocol;
using App::Dsp::FxChain;
using App::Dsp::MasterVolume;
using App::Dsp::OverdriveEffect;
using App::Dsp::DelayEffect;
using App::Dsp::EmptyEffect;
using App::Comm::UsbRx;
using App::Comm::UsbTx;
using App::Comm::CommandDispatcher;

namespace {

std::vector<std::vector<uint8_t>> g_txChunks;

uint8_t captureTransmit(uint8_t* buf, uint16_t len) {
    g_txChunks.emplace_back(buf, buf + len);
    return 0;
}

/// @brief Test harness: UsbRx + dispatcher + UsbTx (capture transport).
struct ControlRxFixture {
    FxChain chain{SAMPLE_RATE_HZ};
    UsbRx usbRx{};
    UsbTx usbTx{&captureTransmit};
    CommandDispatcher dispatcher{chain, usbTx};

    ControlRxFixture() { g_txChunks.clear(); }

    void feed(const uint8_t* data, std::size_t length) {
        usbRx.onBytesReceived(data, length);
    }

    void process() {
        while (auto pkt = usbRx.popPacket()) {
            dispatcher.dispatch(*pkt);
        }
    }

    void pumpTxUntilSyncDone() {
        for (int i = 0; i < 512; ++i) {
            usbTx.sendNext();
            if (!usbTx.isSyncActive()) {
                return;
            }
        }
        FAIL() << "GetState sync did not finish";
    }
};

} // namespace

TEST(ProtocolTest, PacketValidationSuccess) {
    ControlPacket pkt{};
    pkt.sof = 0xA5;
    pkt.command = Command::SetParam;
    pkt.slotId = 1;
    pkt.paramId = 2;
    pkt.setValue(0.75f);
    pkt.applyCRC();

    EXPECT_TRUE(pkt.isValid());
    EXPECT_FLOAT_EQ(pkt.value(), 0.75f);
}

TEST(ProtocolTest, PacketValidationInvalidSOF) {
    ControlPacket pkt{};
    pkt.sof = 0xFF;
    pkt.command = Command::SetParam;
    pkt.applyCRC();

    EXPECT_FALSE(pkt.isValid());
}

TEST(ProtocolTest, PacketValidationInvalidCRC) {
    ControlPacket pkt{};
    pkt.sof = 0xA5;
    pkt.command = Command::SetParam;
    pkt.setValue(1.0f);
    pkt.crc = 0x00;

    EXPECT_FALSE(pkt.isValid());
}

TEST(ControlRxTest, ParseValidByteStreamAndApplyParam) {
    ControlRxFixture fx;
    fx.chain.setEffectByIndex(0, 2);

    ControlPacket originalPkt{};
    originalPkt.command = Command::SetParam;
    originalPkt.slotId = 0;
    originalPkt.paramId = 0;
    originalPkt.setValue(8.5f);
    originalPkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPkt);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    std::visit([](auto& effect) {
        using T = std::decay_t<decltype(effect)>;
        if constexpr (std::is_same_v<T, OverdriveEffect>) {
            EXPECT_FLOAT_EQ(effect.paramValue(0), 8.5f);
        } else {
            FAIL() << "Expected OverdriveEffect in slot 0";
        }
    }, fx.chain.slot(0));
}

TEST(ControlRxTest, IgnoreNoiseBeforeSOF) {
    ControlRxFixture fx;
    fx.chain.setEffectByIndex(0, 1);

    ControlPacket originalPkt{};
    originalPkt.command = Command::SetParam;
    originalPkt.slotId = 0;
    originalPkt.paramId = 1;
    originalPkt.setValue(0.4f);
    originalPkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPkt);
    const uint8_t noiseBytes[] = {0x12, 0x34, 0xFF, 0x00};
    fx.feed(noiseBytes, sizeof(noiseBytes));
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    std::visit([](auto& effect) {
        using T = std::decay_t<decltype(effect)>;
        if constexpr (std::is_same_v<T, DelayEffect>) {
            EXPECT_FLOAT_EQ(effect.paramValue(1), 0.4f);
        } else {
            FAIL() << "Expected DelayEffect in slot 0";
        }
    }, fx.chain.slot(0));
}

TEST(ControlRxTest, ClearSlotCommand) {
    ControlRxFixture fx;
    fx.chain.setEffectByIndex(0, 2);
    EXPECT_FALSE(std::holds_alternative<EmptyEffect>(fx.chain.slot(0)));

    ControlPacket pkt{};
    pkt.command = Command::ClearSlot;
    pkt.slotId = 0;
    pkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&pkt);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    EXPECT_TRUE(std::holds_alternative<EmptyEffect>(fx.chain.slot(0)));
}

TEST(ControlRxTest, SetMasterVolumeCommand) {
    ControlRxFixture fx;

    ControlPacket pkt{};
    pkt.command = Command::SetMasterVolume;
    pkt.setValue(0.55f);
    pkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&pkt);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    EXPECT_FLOAT_EQ(fx.chain.masterGain(), 0.55f);
}

TEST(ControlRxTest, GetStateSyncViaUsbTx) {
    ControlRxFixture fx;
    fx.chain.setEffectByIndex(0, 2);

    ControlPacket getState{};
    getState.command = Command::GetState;
    getState.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&getState);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    EXPECT_TRUE(fx.usbTx.isSyncActive());
    fx.pumpTxUntilSyncDone();
    EXPECT_FALSE(fx.usbTx.isSyncActive());

    ASSERT_FALSE(g_txChunks.empty());
    ASSERT_EQ(g_txChunks.front().size(), sizeof(ControlPacket));

    ControlPacket marker{};
    std::memcpy(&marker, g_txChunks.front().data(), sizeof(marker));
    EXPECT_EQ(marker.command, Command::ManifestFollows);

    bool sawEffectType = false;
    bool sawMasterVolume = false;
    for (const auto& chunk : g_txChunks) {
        if (chunk.size() != sizeof(ControlPacket)) {
            continue; // manifest text slice
        }
        ControlPacket pkt{};
        std::memcpy(&pkt, chunk.data(), sizeof(pkt));
        if (!pkt.isValid()) {
            continue;
        }
        if (pkt.command == Command::ReportEffectType && pkt.slotId == 0) {
            sawEffectType = true;
            EXPECT_EQ(pkt.paramId, 2);
        }
        if (pkt.command == Command::ReportMasterVolume) {
            sawMasterVolume = true;
            EXPECT_FLOAT_EQ(pkt.value(), MasterVolume::kDefaultGain);
        }
    }
    EXPECT_TRUE(sawEffectType);
    EXPECT_TRUE(sawMasterVolume);

    bool sawNoneInManifest = false;
    for (const auto& chunk : g_txChunks) {
        if (chunk.size() == sizeof(ControlPacket)) {
            continue;
        }
        const std::string text(chunk.begin(), chunk.end());
        if (text.find("E:0:None") != std::string::npos) {
            sawNoneInManifest = true;
        }
    }
    EXPECT_TRUE(sawNoneInManifest);
}

TEST(ControlRxTest, SetEffectTypeEnqueuesSlotReportsWhenIdle) {
    ControlRxFixture fx;

    ControlPacket setType{};
    setType.command = Command::SetEffectType;
    setType.slotId = 0;
    setType.effectTypeId = OverdriveEffect::Id;
    setType.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&setType);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    g_txChunks.clear();
    for (int i = 0; i < 16; ++i) {
        fx.usbTx.sendNext();
    }

    bool sawType = false;
    bool sawBypass = false;
    int paramReports = 0;
    for (const auto& chunk : g_txChunks) {
        ASSERT_EQ(chunk.size(), sizeof(ControlPacket));
        ControlPacket pkt{};
        std::memcpy(&pkt, chunk.data(), sizeof(pkt));
        ASSERT_TRUE(pkt.isValid());
        if (pkt.command == Command::ReportEffectType) {
            sawType = true;
            EXPECT_EQ(pkt.slotId, 0);
            EXPECT_EQ(pkt.effectTypeId, OverdriveEffect::Id);
        } else if (pkt.command == Command::ReportBypass) {
            sawBypass = true;
            EXPECT_EQ(pkt.slotId, 0);
            EXPECT_FLOAT_EQ(pkt.value(), 0.0f);
        } else if (pkt.command == Command::ReportParam) {
            ++paramReports;
            EXPECT_EQ(pkt.slotId, 0);
        }
    }
    EXPECT_TRUE(sawType);
    EXPECT_TRUE(sawBypass);
    EXPECT_EQ(paramReports, OverdriveEffect::ParamCount);
}

TEST(ControlRxTest, SetParamDoesNotEnqueueReport) {
    ControlRxFixture fx;
    fx.chain.setEffectByIndex(0, OverdriveEffect::Id);

    ControlPacket setParam{};
    setParam.command = Command::SetParam;
    setParam.slotId = 0;
    setParam.paramId = 0;
    setParam.setValue(7.0f);
    setParam.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&setParam);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    g_txChunks.clear();
    for (int i = 0; i < 8; ++i) {
        fx.usbTx.sendNext();
    }
    EXPECT_TRUE(g_txChunks.empty());
}

TEST(ControlRxTest, BypassToggleSetsAbsoluteAndReports) {
    ControlRxFixture fx;
    fx.chain.setEffectByIndex(0, DelayEffect::Id);

    ControlPacket bypass{};
    bypass.command = Command::BypassToggle;
    bypass.slotId = 0;
    bypass.setValue(1.0f);
    bypass.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&bypass);
    fx.feed(bytes, sizeof(ControlPacket));
    fx.process();

    std::visit([](auto& effect) {
        EXPECT_TRUE(effect.isBypassed());
    }, fx.chain.slot(0));

    g_txChunks.clear();
    for (int i = 0; i < 4; ++i) {
        fx.usbTx.sendNext();
    }

    ASSERT_EQ(g_txChunks.size(), 1u);
    ControlPacket report{};
    std::memcpy(&report, g_txChunks.front().data(), sizeof(report));
    EXPECT_TRUE(report.isValid());
    EXPECT_EQ(report.command, Command::ReportBypass);
    EXPECT_FLOAT_EQ(report.value(), 1.0f);
}
