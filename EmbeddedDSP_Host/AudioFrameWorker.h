#ifndef EMBEDDEDDSP_HOST_AUDIOFRAMEWORKER_H
#define EMBEDDEDDSP_HOST_AUDIOFRAMEWORKER_H

#include <QObject>
#include <memory>
#include <vector>

namespace Protocol { struct AudioFramePacket; }

namespace Host {

class FftProcessor;

/// @brief Processes raw audio frames and coordinates analysis tasks in a worker thread.
class AudioFrameWorker : public QObject {
    Q_OBJECT
public:
    explicit AudioFrameWorker(QObject* parent = nullptr);
    ~AudioFrameWorker() override;

public slots:
    void processFrame(const Protocol::AudioFramePacket& frame);

signals:
    void spectrumReady(const std::vector<float>& magnitudeDb);

private:
    std::unique_ptr<FftProcessor> m_fftProcessor;
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_AUDIOFRAMEWORKER_H
