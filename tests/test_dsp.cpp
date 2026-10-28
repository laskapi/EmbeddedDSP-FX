#include <gtest/gtest.h>
#include "DSP/OverdriveEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/EffectChain.h"
#include "Protocol/ProtocolCommon.h"

TEST(OverdriveTest, ClippingLogic) {
    OverdriveEffect overdrive{Protocol::SAMPLE_RATE_HZ};
    overdrive.setParamValue(0, 10.0f); // High Gain
    overdrive.setParamValue(1, 1.0f);  // Max Tone (cutoff high)

    float left = 1.0f;
    float right = 1.0f;

    // "Warm up" the internal IIR filter state to reach steady state
    for (int i = 0; i < 500; ++i) {
        float l = 1.0f;
        float r = 1.0f;
        overdrive.process(l, r);
    }

    overdrive.process(left, right);

    // For gain 10 and input 1.0, it should be hard-clipped to exactly 2/3 (approx 0.666)
    EXPECT_LT(left, 0.7f);
    EXPECT_GT(left, 0.6f);
    EXPECT_NEAR(left, right, 0.0001f);
}

TEST(DelayTest, SignalPassThroughAndFeedback) {
    DelayEffect delay{Protocol::SAMPLE_RATE_HZ};
    delay.setParamValue(0, 0.1f); // 100ms delay
    delay.setParamValue(1, 0.5f); // 50% feedback
    delay.setParamValue(2, 1.0f); // 100% wet

    float left = 1.0f;
    float right = 1.0f;

    // First sample - output should be 0 because buffer is empty (100% wet)
    delay.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.0f);

    // Test bypass functionality
    delay.toggleBypass();
    left = 0.5f;
    delay.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.5f); // Bypassed should pass clean signal
}

TEST(EffectChainTest, SlotManagement) {
    EffectChain chain{Protocol::SAMPLE_RATE_HZ};

    // Initially all slots should be EmptyEffect (ID 0)
    std::visit([](auto& fx) {
        EXPECT_EQ(std::decay_t<decltype(fx)>::Id, 0);
    }, chain.slot(0));

    // Set slot 0 to Overdrive (ID 2)
    chain.setEffectByIndex(0, 2);

    std::visit([](auto& fx) {
        EXPECT_EQ(std::decay_t<decltype(fx)>::Id, 2);
    }, chain.slot(0));
}
