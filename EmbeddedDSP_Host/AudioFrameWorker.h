#ifndef EMBEDDEDDSP_HOST_AUDIOFRAMEWORKER_H
#define EMBEDDEDDSP_HOST_AUDIOFRAMEWORKER_H

#include <QObject>
#include <memory>
#include <vector>

namespace Protocol { struct AudioFramePacket; }

namespace Host {

class FftProcessor;

/**
 * @brief High-level audio frame worker living in the dedicated worker thread.
 * Processes raw audio frames and coordinates various analysis tasks (e.g., FFT).
 */
class AudioFrameWorker : public QObject {
    Q_OBJECT
public:
    explicit AudioFrameWorker(QObject* parent = nullptr);
    ~AudioFrameWorker() override;

public slots:
    /// @brief Ingests a raw audio frame and triggers the processing pipeline.
    void processFrame(const Protocol::AudioFramePacket& frame);

signals:
    /// @brief Emitted when a new frequency spectrum (in dB) is calculated.
    void spectrumReady(const std::vector<float>& magnitudeDb);

private:
    std::unique_ptr<FftProcessor> m_fftProcessor;
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_AUDIOFRAMEWORKER_H
