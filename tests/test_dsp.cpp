#include <gtest/gtest.h>
#include "DSP/OverdriveEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/FxChain.h"
#include "Protocol/ProtocolCommon.h"

using App::Dsp::OverdriveEffect;
using App::Dsp::DelayEffect;
using App::Dsp::FxChain;

TEST(OverdriveTest, ClippingLogic) {
    OverdriveEffect overdrive{Protocol::SAMPLE_RATE_HZ};
    overdrive.setParamValue(0, 10.0f); // high gain
    overdrive.setParamValue(1, 1.0f);  // open tone

    float left = 1.0f;
    float right = 1.0f;

    // Warm up IIR state before measuring steady-state clip level.
    for (int i = 0; i < 500; ++i) {
        float l = 1.0f;
        float r = 1.0f;
        overdrive.process(l, r);
    }

    overdrive.process(left, right);

    // Hard clip asymptote ≈ 2/3 for this drive curve.
    EXPECT_LT(left, 0.7f);
    EXPECT_GT(left, 0.6f);
    EXPECT_NEAR(left, right, 0.0001f);
}

TEST(DelayTest, SignalPassThroughAndFeedback) {
    DelayEffect delay{Protocol::SAMPLE_RATE_HZ};
    delay.setParamValue(0, 0.1f);
    delay.setParamValue(1, 0.5f);
    delay.setParamValue(2, 1.0f); // 100% wet

    float left = 1.0f;
    float right = 1.0f;

    // Empty delay line + full wet → first output is silence.
    delay.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.0f);

    delay.toggleBypass();
    left = 0.5f;
    delay.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.5f);
}

TEST(FxChainTest, SlotManagement) {
    FxChain chain{Protocol::SAMPLE_RATE_HZ};

    std::visit([](auto& fx) {
        EXPECT_EQ(std::decay_t<decltype(fx)>::Id, 0);
    }, chain.slot(0));

    chain.setEffectByIndex(0, 2);

    std::visit([](auto& fx) {
        EXPECT_EQ(std::decay_t<decltype(fx)>::Id, 2);
    }, chain.slot(0));
}
