#include <gtest/gtest.h>
#include <cmath>
#include "DSP/OverdriveEffect.h"
#include "DSP/DelayEffect.h"
#include "DSP/TremoloEffect.h"
#include "DSP/BitcrusherEffect.h"
#include "DSP/EqEffect.h"
#include "DSP/FxChain.h"
#include "Dsp/BiquadCoeffs.h"
#include "Dsp/EqToneStack.h"
#include "Protocol/ProtocolCommon.h"

using App::Dsp::OverdriveEffect;
using App::Dsp::DelayEffect;
using App::Dsp::TremoloEffect;
using App::Dsp::BitcrusherEffect;
using App::Dsp::EqEffect;
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

    // Hard clip asymptote ≈ 2/3, then × default Level 0.7 → ≈ 0.467.
    EXPECT_NEAR(left, (2.0f / 3.0f) * 0.7f, 0.02f);
    EXPECT_NEAR(left, right, 0.0001f);
}

TEST(OverdriveTest, LevelScalesOutputAfterClip) {
    OverdriveEffect od{Protocol::SAMPLE_RATE_HZ};
    od.setParamValue(0, 20.0f);
    od.setParamValue(1, 1.0f);
    od.setParamValue(2, 0.5f);

    for (int i = 0; i < 500; ++i) {
        float l = 1.0f;
        float r = 1.0f;
        od.process(l, r);
    }

    float left = 1.0f;
    float right = 1.0f;
    od.process(left, right);
    EXPECT_NEAR(left, (2.0f / 3.0f) * 0.5f, 0.02f);
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

TEST(FxChainTest, SettingsRoundTrip) {
    FxChain chain{Protocol::SAMPLE_RATE_HZ};
    chain.setEffectByIndex(1, DelayEffect::Id);
    chain.setMasterGain(0.47f);
    std::visit([](auto& fx) {
        fx.setBypassed(true);
        fx.setParamValue(0, 0.22f);
    }, chain.slot(1));

    const auto snap = chain.captureSettings();
    EXPECT_EQ(snap.magic, App::Dsp::ChainSettings::kMagic);

    FxChain restored{Protocol::SAMPLE_RATE_HZ};
    ASSERT_TRUE(restored.applySettings(snap));
    EXPECT_FLOAT_EQ(restored.masterGain(), 0.47f);
    EXPECT_TRUE(std::holds_alternative<DelayEffect>(restored.slot(1)));
    std::visit([](auto& fx) {
        EXPECT_TRUE(fx.isBypassed());
        EXPECT_FLOAT_EQ(fx.paramValue(0), 0.22f);
    }, restored.slot(1));
}

TEST(FxChainTest, RejectsSecondDelay) {
    FxChain chain{Protocol::SAMPLE_RATE_HZ};
    chain.setEffectByIndex(0, DelayEffect::Id);
    chain.setEffectByIndex(1, DelayEffect::Id);

    EXPECT_TRUE(std::holds_alternative<DelayEffect>(chain.slot(0)));
    EXPECT_FALSE(std::holds_alternative<DelayEffect>(chain.slot(1)));

    // Same-slot Delay → Delay stays allowed.
    chain.setEffectByIndex(0, DelayEffect::Id);
    EXPECT_TRUE(std::holds_alternative<DelayEffect>(chain.slot(0)));
}

TEST(TremoloTest, DepthZeroIsUnityGain) {
    TremoloEffect trem{Protocol::SAMPLE_RATE_HZ};
    trem.setParamValue(0, 4.0f);
    trem.setParamValue(1, 0.0f);

    float left = 0.5f;
    float right = -0.25f;
    trem.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.5f);
    EXPECT_FLOAT_EQ(right, -0.25f);
}

TEST(BitcrusherTest, MixZeroIsDry) {
    BitcrusherEffect crush{Protocol::SAMPLE_RATE_HZ};
    crush.setParamValue(0, 4.0f);
    crush.setParamValue(1, 8.0f);
    crush.setParamValue(3, 0.0f); // Mix

    float left = 0.37f;
    float right = -0.37f;
    crush.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.37f);
    EXPECT_FLOAT_EQ(right, -0.37f);
}

TEST(EqTest, FlatGainsAreNearUnity) {
    EqEffect eq{Protocol::SAMPLE_RATE_HZ};
    // Defaults are 0 dB → identity coeffs.
    float left = 0.25f;
    float right = -0.25f;
    eq.process(left, right);
    EXPECT_FLOAT_EQ(left, 0.25f);
    EXPECT_FLOAT_EQ(right, -0.25f);
}

TEST(BiquadCoeffsTest, IdentityMagnitudeIsFlat) {
    using Shared::Dsp::BiquadCoeffs;
    const auto c = BiquadCoeffs::identity();
    const float fs = Protocol::SAMPLE_RATE_HZ;
    EXPECT_NEAR(c.magnitudeDb(100.0f, fs), 0.0f, 0.01f);
    EXPECT_NEAR(c.magnitudeDb(1000.0f, fs), 0.0f, 0.01f);
    EXPECT_NEAR(c.magnitudeDb(8000.0f, fs), 0.0f, 0.01f);
}

TEST(BiquadCoeffsTest, PeakingBoostNearCenterFreq) {
    using Shared::Dsp::BiquadCoeffs;
    const float fs = Protocol::SAMPLE_RATE_HZ;
    const float f0 = 1000.0f;
    const auto c = BiquadCoeffs::peaking(fs, f0, 6.0f, 1.0f);
    EXPECT_NEAR(c.magnitudeDb(f0, fs), 6.0f, 0.15f);
    EXPECT_LT(std::fabs(c.magnitudeDb(100.0f, fs)), 1.5f);
}

TEST(EqToneStackTest, FlatParamsNearZeroDb) {
    const float fs = Protocol::SAMPLE_RATE_HZ;
    EXPECT_NEAR(Shared::Dsp::eqCascadeMagnitudeDb(fs, 0.0f, 0.0f, 0.0f, 1000.0f, 0.7f, 1000.0f),
                0.0f, 0.05f);
}

TEST(EqToneStackTest, MidBoostNearCenter) {
    const float fs = Protocol::SAMPLE_RATE_HZ;
    const float db = Shared::Dsp::eqCascadeMagnitudeDb(fs, 0.0f, 6.0f, 0.0f, 1000.0f, 1.0f, 1000.0f);
    EXPECT_NEAR(db, 6.0f, 0.2f);
}

TEST(EqTest, MidQParamRoundTrip) {
    EqEffect eq{Protocol::SAMPLE_RATE_HZ};
    eq.setParamValue(4, 2.5f);
    EXPECT_FLOAT_EQ(eq.paramValue(4), 2.5f);
    eq.setParamValue(4, 0.1f); // below min → clamp
    EXPECT_FLOAT_EQ(eq.paramValue(4), EqEffect::Params[4].min);
}

TEST(FxChainTest, CanPlaceTremoloAndBitcrusher) {
    FxChain chain{Protocol::SAMPLE_RATE_HZ};
    chain.setEffectByIndex(0, TremoloEffect::Id);
    chain.setEffectByIndex(1, BitcrusherEffect::Id);
    EXPECT_TRUE(std::holds_alternative<TremoloEffect>(chain.slot(0)));
    EXPECT_TRUE(std::holds_alternative<BitcrusherEffect>(chain.slot(1)));
}

TEST(FxChainTest, CanPlaceEq) {
    FxChain chain{Protocol::SAMPLE_RATE_HZ};
    chain.setEffectByIndex(2, EqEffect::Id);
    EXPECT_TRUE(std::holds_alternative<EqEffect>(chain.slot(2)));
}
