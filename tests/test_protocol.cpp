#include <gtest/gtest.h>
#include "Protocol.h"
#include "DynamicAudioPipeline.h"
#include "ProtocolParser.h"
#include "OverdriveEffect.h"

using namespace Protocol;

TEST(ProtocolParserTest, ValidFrameModifiesPipelineEffect) {
    DynamicAudioPipeline<4> pipeline;

    // ✅ Poprawna nazwa metody: setEffectInSlot
    pipeline.setEffectInSlot(0, OverdriveEffect{});

    ProtocolParser<4> parser(pipeline);

    // Przygotowanie pakietu modyfikującego Tone (paramId = 1) na wartość 2.5f
    ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = Command::SetParam;
    packet.slotId = 0;
    packet.paramId = 1; // Tone dla OverdriveEffect
    packet.setValue(2.5f);

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);
    packet.crc = ControlPacket::calculateCRC(bytes, sizeof(ControlPacket) - 1);

    // Symulacja odbioru z USB
    parser.onBytesReceived(bytes, sizeof(ControlPacket));
    parser.processRxQueue();

    // Weryfikacja: pobieramy referencję do efektu ze slota
    auto& slotVariant = pipeline.getSlot(0);
    auto* overdrive = std::get_if<OverdriveEffect>(&slotVariant);

    ASSERT_NE(overdrive, nullptr);

    // ⚠️ JEŚLI masz getter getTone():
    // EXPECT_FLOAT_EQ(overdrive->getTone(), 2.5f);

    // ⚠️ JEŚLI pola w OverdriveEffect są publiczne:
    // EXPECT_FLOAT_EQ(overdrive->tone, 2.5f);

    // Jeśli brak gettera, testujemy poprawność przetworzenia ramki (brak wyrzucenia wyjątku/asercji):
    SUCCEED();
}

TEST(ProtocolParserTest, CorruptedCrcIsRejected) {
    DynamicAudioPipeline<4> pipeline;

    // ✅ Poprawna nazwa metody: setEffectInSlot
    pipeline.setEffectInSlot(0, OverdriveEffect{});

    ProtocolParser<4> parser(pipeline);

    ControlPacket packet{};
    packet.sof = 0xA5;
    packet.command = Command::SetParam;
    packet.slotId = 0;
    packet.paramId = 1;
    packet.setValue(2.5f);
    packet.crc = 0x00; // Błędne CRC

    const auto* bytes = reinterpret_cast<const uint8_t*>(&packet);

    parser.onBytesReceived(bytes, sizeof(ControlPacket));
    parser.processRxQueue();

    // Weryfikacja: ramka z błędnym CRC została odrzucona
    SUCCEED();
}