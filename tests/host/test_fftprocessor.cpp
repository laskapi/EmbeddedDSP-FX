#include <gtest/gtest.h>
#include "Processing/FftProcessor.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <numbers>

using namespace Host::Processing;

class FftProcessorTest : public ::testing::Test {
protected:
    const size_t fftSize = 256;
    const float sampleRate = 44100.0f;
};

TEST_F(FftProcessorTest, SilenceResultsInMinimumMagnitude) {
    FftProcessor processor(fftSize);
    std::vector<int16_t> input(fftSize, 0);

    const auto& spectrum = processor.processFrame(input);

    // Silence maps to the processor dB floor.
    for (float db : spectrum) {
        EXPECT_NEAR(db, -100.0f, 0.1f);
    }
}

TEST_F(FftProcessorTest, DCOffsetProducesPeakAtBinZero) {
    FftProcessor processor(fftSize);
    std::vector<int16_t> input(fftSize, 32767);

    const auto& spectrum = processor.processFrame(input);

    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    int peakIndex = std::distance(spectrum.begin(), maxIt);

    EXPECT_EQ(peakIndex, 0);
    EXPECT_GT(*maxIt, -10.0f);
}

TEST_F(FftProcessorTest, DetectsSineWavePeak) {
    FftProcessor processor(fftSize);
    const float frequency = 440.0f;
    std::vector<int16_t> input(fftSize);

    for (size_t i = 0; i < fftSize; ++i) {
        float time = static_cast<float>(i) / sampleRate;
        float sample = std::sin(2.0f * std::numbers::pi_v<float> * frequency * time);
        input[i] = static_cast<int16_t>(sample * 32767.0f);
    }

    const auto& spectrum = processor.processFrame(input);

    // bin ≈ f / (fs / N) = 440 / (44100 / 256) ≈ 2.55
    auto maxIt = std::max_element(spectrum.begin(), spectrum.end());
    int peakIndex = std::distance(spectrum.begin(), maxIt);

    EXPECT_TRUE(peakIndex == 2 || peakIndex == 3);
    EXPECT_GT(*maxIt, -20.0f);

    if (spectrum.size() > 10) {
        EXPECT_LT(spectrum[10], -40.0f);
    }
}

TEST_F(FftProcessorTest, PushSamplesEmitsOnHopAfterFill) {
    constexpr size_t kFft = 256;
    constexpr size_t kHop = 128;
    FftProcessor processor(kFft, kHop);
    std::vector<int16_t> chunk(64, 1000);

    int emissions = 0;
    // First spectrum only after a full window is filled.
    for (size_t n = 0; n < kFft / chunk.size(); ++n) {
        if (processor.pushSamples(chunk) != nullptr) {
            ++emissions;
        }
    }
    EXPECT_EQ(emissions, 1);

    emissions = 0;
    for (size_t n = 0; n < kHop / chunk.size(); ++n) {
        if (processor.pushSamples(chunk) != nullptr) {
            ++emissions;
        }
    }
    EXPECT_EQ(emissions, 1);
}
