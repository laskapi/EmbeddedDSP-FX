#include <gtest/gtest.h>
#include "Models/DeviceManifestParser.h"

using Host::Models::parseDeviceManifest;

TEST(ManifestParserTest, ParsesEmptyEffectWithoutParams) {
    const QString text =
        "C:SLOTS:4\n"
        "E:0:None\n"
        "E:1:Delay;P:Time:0.01:1.00:0.50:s\n";

    const auto manifest = parseDeviceManifest(text);

    EXPECT_EQ(manifest.slotCount, 4);
    ASSERT_TRUE(manifest.availableEffects.contains(0));
    EXPECT_EQ(manifest.availableEffects[0].name, QStringLiteral("None"));
    EXPECT_TRUE(manifest.availableEffects[0].params.isEmpty());
    ASSERT_TRUE(manifest.availableEffects.contains(1));
    ASSERT_EQ(manifest.availableEffects[1].params.size(), 1);
    EXPECT_EQ(manifest.availableEffects[1].params[0].name, QStringLiteral("Time"));
    EXPECT_EQ(manifest.availableEffects[1].params[0].unit, QStringLiteral("s"));
}

TEST(ManifestParserTest, ParsesSlotsAndOneEffect) {
    const QString text =
        "C:SLOTS:4\n"
        "E:2:Overdrive;P:Gain:1.00:20.00:5.00:;P:Tone:0.10:1.00:0.50:\n";

    const auto manifest = parseDeviceManifest(text);

    EXPECT_EQ(manifest.slotCount, 4);
    ASSERT_TRUE(manifest.availableEffects.contains(2));
    EXPECT_EQ(manifest.availableEffects[2].name, QStringLiteral("Overdrive"));
    ASSERT_EQ(manifest.availableEffects[2].params.size(), 2);
    EXPECT_FLOAT_EQ(manifest.availableEffects[2].params[0].max, 20.0f);
    EXPECT_TRUE(manifest.availableEffects[2].params[0].unit.isEmpty());
}

TEST(ManifestParserTest, LeavesSlotCountZeroWhenConfigLineMissing) {
    const QString text = QStringLiteral("E:1:Delay;P:Time:10.00:1000.00:350.00:\n");

    const auto manifest = parseDeviceManifest(text);

    EXPECT_EQ(manifest.slotCount, 0);
    ASSERT_EQ(manifest.availableEffects.size(), 1);
    EXPECT_EQ(manifest.availableEffects[1].name, QStringLiteral("Delay"));
}

TEST(ManifestParserTest, EmptyInputLeavesZeroSlotsAndNoEffects) {
    const auto manifest = parseDeviceManifest(QString());

    EXPECT_EQ(manifest.slotCount, 0);
    EXPECT_TRUE(manifest.availableEffects.isEmpty());
}

TEST(ManifestParserTest, IgnoresUnrecognizedLines) {
    const auto manifest = parseDeviceManifest(QStringLiteral("not-a-manifest-line\n"));

    EXPECT_EQ(manifest.slotCount, 0);
    EXPECT_TRUE(manifest.availableEffects.isEmpty());
}

TEST(ManifestParserTest, ParsesMultipleEffects) {
    const QString text =
        "C:SLOTS:2\n"
        "E:1:Overdrive;P:Gain:1.00:20.00:5.00:\n"
        "E:2:Delay;P:Time:10.00:1000.00:350.00:\n";

    const auto manifest = parseDeviceManifest(text);

    EXPECT_EQ(manifest.slotCount, 2);
    EXPECT_EQ(manifest.availableEffects.size(), 2);
    EXPECT_TRUE(manifest.availableEffects.contains(1));
    EXPECT_TRUE(manifest.availableEffects.contains(2));
}

TEST(ManifestParserTest, TrustsSlotCountFromFirmware) {
    const auto manifest = parseDeviceManifest(QStringLiteral("C:SLOTS:99\n"));
    EXPECT_EQ(manifest.slotCount, 99);
}
