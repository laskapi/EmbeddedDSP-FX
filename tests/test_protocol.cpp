#include <gtest/gtest.h>
#include "Protocol.h"
#include "ProtocolParser.h"
#include "DynamicAudioPipeline.h"

TEST(ProtocolTest, PacketValidationSuccess) {
    Protocol::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = Protocol::Command::SetParam;
    packet.slotId = 1;
    packet.paramId = 2;
    packet.setValue(0.75f);

    // Calculate CRC over the first 8 bytes (excluding the CRC field itself)
    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = Protocol::ControlPacket::calculateCRC(bytes, sizeof(Protocol::ControlPacket) - 1);

    EXPECT_TRUE(packet.isValid());
    EXPECT_FLOAT_EQ(packet.getValue(), 0.75f);
}

TEST(ProtocolTest, PacketValidationInvalidSOF) {
    Protocol::ControlPacket packet{};
    packet.sof = 0xFF; // Invalid SOF byte
    packet.command = Protocol::Command::SetParam;
    packet.slotId = 0;
    packet.paramId = 0;
    packet.setValue(1.0f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = Protocol::ControlPacket::calculateCRC(bytes, sizeof(Protocol::ControlPacket) - 1);

    EXPECT_FALSE(packet.isValid());
}

TEST(ProtocolTest, PacketValidationInvalidCRC) {
    Protocol::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = Protocol::Command::SetParam;
    packet.slotId = 0;
    packet.paramId = 0;
    packet.setValue(1.0f);
    packet.crc = 0x00; // Corrupted CRC

    EXPECT_FALSE(packet.isValid());
}

TEST(ProtocolParserTest, ParseValidByteStreamAndApplyParam) {
    DynamicAudioPipeline pipeline{};
    ProtocolParser parser{pipeline};

    // Correctly instantiate and assign Overdrive to slot 0
    pipeline.setEffectInSlot(0, OverdriveEffect{});

    Protocol::ControlPacket originalPacket{};
    originalPacket.sof = 0xA5;
    originalPacket.command = Protocol::Command::SetParam;
    originalPacket.slotId = 0;
    originalPacket.paramId = 0; // Drive parameter
    originalPacket.setValue(8.5f); // Valid value within range [1.0f, 20.0f]

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPacket);
    originalPacket.crc = Protocol::ControlPacket::calculateCRC(bytes, sizeof(Protocol::ControlPacket) - 1);

    // Simulate byte reception from USB ISR
    parser.onBytesReceived(bytes, sizeof(Protocol::ControlPacket));

    // Process queued bytes in main loop execution
    parser.processRxQueue();

    // Verify parameter was correctly applied via visitor/variant
    std::visit([](auto& effect) {
        using T = std::decay_t<decltype(effect)>;
        if constexpr (std::is_same_v<T, OverdriveEffect>) {
            EXPECT_FLOAT_EQ(effect.getDrive(), 8.5f);
        } else {
            FAIL() << "Expected OverdriveEffect in slot 0";
        }
    }, pipeline.getSlot(0));
}

TEST(ProtocolParserTest, IgnoreNoiseBeforeSOF) {
    DynamicAudioPipeline pipeline{};
    ProtocolParser parser{pipeline};

    // Correctly instantiate and assign Delay to slot 0
    pipeline.setEffectInSlot(0, DelayEffect{});

    Protocol::ControlPacket originalPacket{};
    originalPacket.sof = 0xA5;
    originalPacket.command = Protocol::Command::SetParam;
    originalPacket.slotId = 0;
    originalPacket.paramId = 1; // Feedback parameter
    originalPacket.setValue(0.4f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPacket);
    originalPacket.crc = Protocol::ControlPacket::calculateCRC(bytes, sizeof(Protocol::ControlPacket) - 1);

    // Simulate noise bytes before a valid SOF packet
    const uint8_t noiseBytes[] = {0x12, 0x34, 0xFF, 0x00};
    parser.onBytesReceived(noiseBytes, sizeof(noiseBytes));
    parser.onBytesReceived(bytes, sizeof(Protocol::ControlPacket));

    parser.processRxQueue();

    std::visit([](auto& effect) {
        using T = std::decay_t<decltype(effect)>;
        if constexpr (std::is_same_v<T, DelayEffect>) {
            EXPECT_FLOAT_EQ(effect.getFeedback(), 0.4f);
        } else {
            FAIL() << "Expected DelayEffect in slot 0";
        }
    }, pipeline.getSlot(0));
}