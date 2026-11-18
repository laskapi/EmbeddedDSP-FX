#include "Processing/AudioFrameWorker.h"
#include "Processing/FftProcessor.h"
#include "Protocol/AudioFrame.h"
#include "Protocol/ProtocolCommon.h"

#include <span>

namespace Host::Processing {
namespace {
constexpr std::size_t kSpectrumFftSize = 1024;
constexpr std::size_t kSpectrumHopSize = kSpectrumFftSize / 2;
}

AudioFrameWorker::AudioFrameWorker(QObject* parent)
    : QObject(parent)
    , m_fftProcessor(std::make_unique<FftProcessor>(kSpectrumFftSize, kSpectrumHopSize))
{
    static_assert(Protocol::AUDIO_SAMPLES > 0);
    static_assert(kSpectrumFftSize % Protocol::AUDIO_SAMPLES == 0);
}

AudioFrameWorker::~AudioFrameWorker() = default;

void AudioFrameWorker::processFrame(const Protocol::AudioFrame& frame) {
    const std::span<const std::int16_t> pcm{frame.samples.data(), frame.samples.size()};

    if (const std::vector<float>* spectrum = m_fftProcessor->pushSamples(pcm)) {
        emit spectrumReady(*spectrum);
    }
}

} // namespace Host::Processing
