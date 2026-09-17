#include <gtest/gtest.h>
#include <Protocol/ControlPacket.h>
#include <Protocol/ControlParser.h>
#include <DSP/DynamicAudioPipeline.h>

using namespace Protocol;

TEST(ProtocolTest, PacketValidationSuccess) {
    ControlPacket pkt{};
    pkt.sof = 0xA5;
    pkt.command = Command::SetParam;
    pkt.slotId = 1;
    pkt.paramId = 2;
    pkt.setValue(0.75f);
    pkt.applyCRC();

    EXPECT_TRUE(pkt.isValid());
    EXPECT_FLOAT_EQ(pkt.getValue(), 0.75f);
}

TEST(ProtocolTest, PacketValidationInvalidSOF) {
    ControlPacket pkt{};
    pkt.sof = 0xFF; // Invalid SOF byte
    pkt.command = Command::SetParam;
    pkt.applyCRC();

    EXPECT_FALSE(pkt.isValid());
}

TEST(ProtocolTest, PacketValidationInvalidCRC) {
    ControlPacket pkt{};
    pkt.sof = 0xA5;
    pkt.command = Command::SetParam;
    pkt.setValue(1.0f);
    pkt.crc = 0x00; // Corrupted CRC

    EXPECT_FALSE(pkt.isValid());
}

TEST(ProtocolParserTest, ParseValidByteStreamAndApplyParam) {
    DynamicAudioPipeline pipeline{};
    ControlParser parser{pipeline};
    
    // Discovery manifest would normally set this up, but we do it manually for test
    pipeline.setEffectByIndex(0, 2, 48000.0f); // 2 is OverdriveEffect ID

    ControlPacket originalPkt{};
    originalPkt.command = Command::SetParam;
    originalPkt.slotId = 0;
    originalPkt.paramId = 0; // Drive parameter
    originalPkt.setValue(8.5f);
    originalPkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPkt);
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

    ControlPacket originalPkt{};
    originalPkt.command = Command::SetParam;
    originalPkt.slotId = 0;
    originalPkt.paramId = 1; // Feedback parameter
    originalPkt.setValue(0.4f);
    originalPkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&originalPkt);
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

    ControlPacket pkt{};
    pkt.command = Command::ClearSlot; 
    pkt.slotId = 0;
    pkt.applyCRC();

    const auto* bytes = reinterpret_cast<const uint8_t*>(&pkt);
    parser.onBytesReceived(bytes, sizeof(ControlPacket));
    parser.processRxQueue();

    EXPECT_TRUE(std::holds_alternative<EmptyEffect>(pipeline.getSlot(0)));
}
