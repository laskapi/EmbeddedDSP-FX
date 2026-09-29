#include "AudioFrameWorker.h"
#include "FftProcessor.h"
#include <Protocol/AudioFramePacket.h>
#include <span>

namespace Host {

AudioFrameWorker::AudioFrameWorker(QObject* parent) 
    : QObject(parent)
    , m_fftProcessor(std::make_unique<FftProcessor>(Protocol::AUDIO_SAMPLES)) 
{
}

AudioFrameWorker::~AudioFrameWorker() = default;

void AudioFrameWorker::processFrame(const Protocol::AudioFramePacket& frame) {
    const std::span<const int16_t> pcm{frame.samples};
    
    // Primary task: FFT Analysis
    const auto& spectrum = m_fftProcessor->processFrame(pcm);
    emit spectrumReady(spectrum);
    
    // Future expansion: RMS, Peak detection, etc. can be added here
}

} // namespace Host
