#include <gtest/gtest.h>
#include "FftProcessor.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numbers>

using namespace Host::Processing;

/// @brief Tests the FftProcessor's ability to handle various signal types.
class FftProcessorTest : public ::testing::Test {
protected:
    const size_t fftSize = 256;
    const float sampleRate = 44100.0f;
};

TEST_F(FftProcessorTest, SilenceResultsInMinimumMagnitude) {
    FftProcessor processor(fftSize);
    std::vector<int16_t> input(fftSize, 0);

    const auto& spectrum = processor.processFrame(input);

    // With zero input, all bins should be at or near the minimum dB floor (-100 dB)
    for (float db : spectrum) {
        EXPECT_NEAR(db, -100.0f, 0.1f);
    }
}

TEST_F(FftProcessorTest, DCOffsetProducesPeakAtBinZero) {
    FftProcessor processor(fftSize);
    // Constant signal (DC offset) - all samples at max positive value
    std::vector<int16_t> input(fftSize, 32767);

    const auto& spectrum = processor.processFrame(input);

    // Find index of maximum magnitude
    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    int peakIndex = std::distance(spectrum.begin(), maxIt);

    // For a constant signal, the peak must be at bin 0 (0 Hz)
    EXPECT_EQ(peakIndex, 0);
    EXPECT_GT(*maxIt, -10.0f); // Peak should be significant
}

TEST_F(FftProcessorTest, DetectsSineWavePeak) {
    FftProcessor processor(fftSize);
    const float frequency = 440.0f; // Standard A4 note
    std::vector<int16_t> input(fftSize);

    // Generate 440Hz sine wave
    for (size_t i = 0; i < fftSize; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float sample = std::sin(2.0f * std::numbers::pi_v<float> * frequency * time);
        input[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    const auto& spectrum = processor.processFrame(input);

    // Expected bin calculation: f / (fs / N)
    // 440 / (44100 / 256) = 2.55 -> peak should be around bin 2 or 3
    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    int peakIndex = std::distance(spectrum.begin(), maxIt);

    EXPECT_TRUE(peakIndex == 2 || peakIndex == 3);
    EXPECT_GT(*maxIt, -20.0f); // Should be a strong peak
    
    // Check that other bins far from the peak are much lower
    if (spectrum.size() > 10) {
        EXPECT_LT(spectrum[10], -40.0f);
    }
}
