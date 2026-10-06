#include <gtest/gtest.h>
#include <DSP/OverdriveEffect.h>
#include <DSP/DelayEffect.h>
#include <DSP/DynamicAudioPipeline.h>

TEST(OverdriveTest, ClippingLogic) {
    OverdriveEffect overdrive;
    overdrive.prepare(48000.0f);
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
    
    // Now process and check the result
    overdrive.process(left, right);
    
    // For gain 10 and input 1.0, it should be hard-clipped to exactly 2/3 (approx 0.666)
    EXPECT_LT(left, 0.7f);
    EXPECT_GT(left, 0.6f);
    EXPECT_NEAR(left, right, 0.0001f);
}

TEST(DelayTest, SignalPassThroughAndFeedback) {
    DelayEffect delay;
    delay.prepare(48000.0f);
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

TEST(PipelineTest, SlotManagement) {
    DynamicAudioPipeline pipeline;
    pipeline.prepare(48000.0f);
    
    // Initially all slots should be EmptyEffect (ID 0)
    std::visit([](auto& fx) {
        EXPECT_EQ(std::decay_t<decltype(fx)>::Id, 0);
    }, pipeline.getSlot(0));

    // Set slot 0 to Overdrive (ID 2)
    pipeline.setEffectByIndex(0, 2, 48000.0f);
    
    std::visit([](auto& fx) {
        EXPECT_EQ(std::decay_t<decltype(fx)>::Id, 2);
    }, pipeline.getSlot(0));
}
