#include "AudioAnalyzer.h"
#include <span>

namespace Host {

AudioAnalyzer::AudioAnalyzer(QObject* parent) : QObject(parent) {
}

void AudioAnalyzer::processFrame(const Protocol::AudioFramePacket& frame) {
    const std::span<const int16_t> pcm{frame.samples};
    const auto& spectrum = m_fftProcessor.processFrame(pcm);
    emit spectrumReady(spectrum);
}

} // namespace Host
