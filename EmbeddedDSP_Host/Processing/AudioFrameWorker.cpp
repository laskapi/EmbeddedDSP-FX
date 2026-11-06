#include "Processing/AudioFrameWorker.h"
#include "Processing/FftProcessor.h"
#include "Protocol/AudioFrame.h"
#include <span>

namespace Host::Processing {

AudioFrameWorker::AudioFrameWorker(QObject* parent) 
    : QObject(parent)
    , m_fftProcessor(std::make_unique<FftProcessor>(Protocol::AUDIO_SAMPLES)) 
{
}

AudioFrameWorker::~AudioFrameWorker() = default;

void AudioFrameWorker::processFrame(const Protocol::AudioFrame& frame) {
    const std::span<const int16_t> pcm{frame.samples.data(), frame.samples.size()};
    
    const auto& spectrum = m_fftProcessor->processFrame(pcm);
    emit spectrumReady(spectrum);
}

} // namespace Host::Processing
