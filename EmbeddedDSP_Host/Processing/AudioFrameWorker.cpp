#include "AudioFrameWorker.h"
#include "FftProcessor.h"
#include <Protocol/AudioFramePacket.h>
#include <span>

namespace Host::Processing {

AudioFrameWorker::AudioFrameWorker(QObject* parent) 
    : QObject(parent)
    , m_fftProcessor(std::make_unique<FftProcessor>(Protocol::AUDIO_SAMPLES)) 
{
}

AudioFrameWorker::~AudioFrameWorker() = default;

void AudioFrameWorker::processFrame(const Protocol::AudioFramePacket& frame) {
    const std::span<const int16_t> pcm{frame.samples};
    
    const auto& spectrum = m_fftProcessor->processFrame(pcm);
    emit spectrumReady(spectrum);
}

} // namespace Host::Processing
