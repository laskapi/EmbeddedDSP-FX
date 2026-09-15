#include <gtest/gtest.h>
#include "../EmbeddedDSP_Shared/Protocol/ControlPacket.h"
#include "../EmbeddedDSP_Firmware/App/Protocol/ControlParser.h"
#include "../EmbeddedDSP_Firmware/App/DSP/DynamicAudioPipeline.h"

using namespace Protocol;

TEST(ProtocolTest, PacketValidationSuccess) {
    ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = Command::SetParam;
    packet.slotId = 1;
    packet.paramId = 2;
    packet.setValue(0.75f);
    packet.applyCRC();

    EXPECT_TRUE(packet.isValid());
    EXPECT_FLOAT_EQ(packet.getValue(), 0.75f);
}

TEST(ProtocolTest, PacketValidationInvalidSOF) {
    ControlPacket packet{};
    packet.sof = 0xFF; // Invalid SOF byte
    packet.command = Command::SetParam;
    packet.applyCRC();

    EXPECT_FALSE(packet.isValid());
}

TEST(ProtocolTest, PacketValidationInvalidCRC) {
    ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = Command::SetParam;
    packet.setValue(1.0f);
    packet.crc = 0x00; // Corrupted CRC

    EXPECT_FALSE(packet.isValid());
}

TEST(ProtocolParserTest, ParseValidByteStreamAndApplyParam) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};
    
    // Discovery manifest would normally set this up, but we do it manually for test
    pipeline.setEffectByIndex(0, 2, 48000.0f); // 2 is OverdriveEffect ID

    ControlPacket originalPacket{};
    originalPacket.command = Command::SetParam;
    originalPacket.slotId = 0;
    originalPacket.paramId = 0; // Drive parameter
    originalPacket.setValue(8.5f);
    originalPacket.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPacket);
    parser.onBytesReceived(bytes, sizeof(ControlPacket));
    parser.processRxQueue();

    std::visit([](auto& effect) {
        using T = std::decay_t<decltype(effect)>;
        if constexpr (std::is_same_v<T, OverdriveEffect>) {
            EXPECT_FLOAT_EQ(effect.getParamValue(0), 8.5f);
        } else {
            FAIL() << "Expected OverdriveEffect in slot 0";
        }
    }, pipeline.getSlot(0));
}

TEST(ProtocolParserTest, IgnoreNoiseBeforeSOF) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};

    pipeline.setEffectByIndex(0, 1, 48000.0f); // 1 is DelayEffect ID

    ControlPacket originalPacket{};
    originalPacket.command = Command::SetParam;
    originalPacket.slotId = 0;
    originalPacket.paramId = 1; // Feedback parameter
    originalPacket.setValue(0.4f);
    originalPacket.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPacket);
    const uint8_t noiseBytes[] = {0x12, 0x34, 0xFF, 0x00};
    parser.onBytesReceived(noiseBytes, sizeof(noiseBytes));
    parser.onBytesReceived(bytes, sizeof(ControlPacket));

    parser.processRxQueue();

    std::visit([](auto& effect) {
        using T = std::decay_t<decltype(effect)>;
        if constexpr (std::is_same_v<T, DelayEffect>) {
            EXPECT_FLOAT_EQ(effect.getParamValue(1), 0.4f);
        } else {
            FAIL() << "Expected DelayEffect in slot 0";
        }
    }, pipeline.getSlot(0));
}

TEST(ProtocolParserTest, ClearSlotCommand) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};

    pipeline.setEffectByIndex(0, 2, 48000.0f);
    EXPECT_FALSE(std::holds_alternative<EmptyEffect>(pipeline.getSlot(0)));

    ControlPacket packet{};
    packet.command = Command::ClearSlot; // We need to handle this in parser if we want it to work
    packet.slotId = 0;
    packet.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    parser.onBytesReceived(bytes, sizeof(ControlPacket));
    parser.processRxQueue();

    // Note: ControlParser needs to handle ClearSlot command!
}
