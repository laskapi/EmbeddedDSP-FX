#ifndef EMBEDDEDDSP_HOST_AUDIOANALYZER_H
#define EMBEDDEDDSP_HOST_AUDIOANALYZER_H

#include <QObject>
#include <vector>
#include "FftProcessor.h"
#include <Protocol/AudioFramePacket.h>

namespace Host {

/**
 * @brief High-level audio analyzer living in the worker thread.
 * Processes raw audio frames and produces frequency spectrum data.
 */
class AudioAnalyzer : public QObject {
    Q_OBJECT
public:
    explicit AudioAnalyzer(QObject* parent = nullptr);

public slots:
    /// @brief Ingests a raw audio frame and triggers spectral analysis.
    void processFrame(const Protocol::AudioFramePacket& frame);

signals:
    /// @brief Emitted when a new spectrum (in dB) is calculated.
    void spectrumReady(const std::vector<float>& magnitudeDb);

private:
    FftProcessor m_fftProcessor{Protocol::AUDIO_SAMPLES};
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_AUDIOANALYZER_H
