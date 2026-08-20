#include <gtest/gtest.h>
#include "../EmbeddedDSP_Firmware/App/Protocol/ControlPacket.h"
#include "../EmbeddedDSP_Firmware/App/Protocol/ControlParser.h"
#include "../EmbeddedDSP_Firmware/App/DSP/DynamicAudioPipeline.h"

TEST(ProtocolTest, PacketValidationSuccess) {
    ControlPacket::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = ControlPacket::Command::SetParam;
    packet.slotId = 1;
    packet.paramId = 2;
    packet.setValue(0.75f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    EXPECT_TRUE(packet.isValid());
    EXPECT_FLOAT_EQ(packet.getValue(), 0.75f);
}

TEST(ProtocolTest, PacketValidationInvalidSOF) {
    ControlPacket::ControlPacket packet{};
    packet.sof = 0xFF; // Invalid SOF byte
    packet.command = ControlPacket::Command::SetParam;
    packet.slotId = 0;
    packet.paramId = 0;
    packet.setValue(1.0f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    EXPECT_FALSE(packet.isValid());
}

TEST(ProtocolTest, PacketValidationInvalidCRC) {
    ControlPacket::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = ControlPacket::Command::SetParam;
    packet.slotId = 0;
    packet.paramId = 0;
    packet.setValue(1.0f);
    packet.crc = 0x00; // Corrupted CRC

    EXPECT_FALSE(packet.isValid());
}

TEST(ProtocolParserTest, ParseValidByteStreamAndApplyParam) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};

    pipeline.setEffectInSlot(0, OverdriveEffect{});

    ControlPacket::ControlPacket originalPacket{};
    originalPacket.sof = 0xA5;
    originalPacket.command = ControlPacket::Command::SetParam;
    originalPacket.slotId = 0;
    originalPacket.paramId = 0; // Drive parameter
    originalPacket.setValue(8.5f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPacket);
    originalPacket.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    parser.onBytesReceived(bytes, sizeof(ControlPacket::ControlPacket));
    parser.processRxQueue();

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
    ControlParser parser{pipeline};

    pipeline.setEffectInSlot(0, DelayEffect{});

    ControlPacket::ControlPacket originalPacket{};
    originalPacket.sof = 0xA5;
    originalPacket.command = ControlPacket::Command::SetParam;
    originalPacket.slotId = 0;
    originalPacket.paramId = 1; // Feedback parameter
    originalPacket.setValue(0.4f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPacket);
    originalPacket.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    const uint8_t noiseBytes[] = {0x12, 0x34, 0xFF, 0x00};
    parser.onBytesReceived(noiseBytes, sizeof(noiseBytes));
    parser.onBytesReceived(bytes, sizeof(ControlPacket::ControlPacket));

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

TEST(ProtocolParserTest, ClearSlotCommand) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};

    pipeline.setEffectInSlot(0, OverdriveEffect{});
    EXPECT_FALSE(std::holds_alternative<EmptyEffect>(pipeline.getSlot(0)));

    ControlPacket::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = ControlPacket::Command::ClearSlot;
    packet.slotId = 0;

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    parser.onBytesReceived(bytes, sizeof(ControlPacket::ControlPacket));
    parser.processRxQueue();

    EXPECT_TRUE(std::holds_alternative<EmptyEffect>(pipeline.getSlot(0)));
}

TEST(ProtocolParserTest, SwapSlotsCommand) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};

    pipeline.setEffectInSlot(0, OverdriveEffect{});
    pipeline.setEffectInSlot(1, DelayEffect{});

    ControlPacket::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = ControlPacket::Command::SwapSlots;
    packet.slotId = 0;
    packet.paramId = 1; // Target slot to swap with

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    parser.onBytesReceived(bytes, sizeof(ControlPacket::ControlPacket));
    parser.processRxQueue();

    EXPECT_TRUE(std::holds_alternative<DelayEffect>(pipeline.getSlot(0)));
    EXPECT_TRUE(std::holds_alternative<OverdriveEffect>(pipeline.getSlot(1)));
}

TEST(ProtocolParserTest, SetActiveSlotsCommand) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};

    ControlPacket::ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = ControlPacket::Command::SetActiveSlots;
    packet.slotId = 2; // Set active count to 2

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = ControlPacket::ControlPacket::calculateCRC(bytes, sizeof(ControlPacket::ControlPacket) - 1);

    parser.onBytesReceived(bytes, sizeof(ControlPacket::ControlPacket));
    parser.processRxQueue();

    EXPECT_EQ(pipeline.getActiveSlotsCount(), 2);
}