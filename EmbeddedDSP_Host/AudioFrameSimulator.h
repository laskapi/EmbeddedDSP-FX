#ifndef EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H
#define EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H

#include <QTimer>
#include <cstdint>
#include <Protocol/AudioFramePacket.h>

namespace Host {

/**
 * @brief Generates synthetic audio data for testing without hardware.
 */
class AudioFrameSimulator : public QObject
{
    Q_OBJECT

public:
    explicit AudioFrameSimulator(QObject *parent = nullptr);

    [[nodiscard]] bool isRunning() const;

    void setFrequencyHz(float frequencyHz);
    void setAmplitude(float amplitudeNormalized);
    void setIntervalMs(int intervalMs);

    [[nodiscard]] float frequencyHz() const;
    [[nodiscard]] float amplitude() const;

public slots:
    void start();
    void stop();

signals:
    void audioFrameReady(const Protocol::AudioFramePacket &frame);
    void runningChanged(bool running);

private slots:
    void onTick();

private:
    QTimer m_timer;
    uint8_t m_sequence{0};
    float m_phase{0.0f};
    float m_frequencyHz{1000.0f};
    float m_amplitude{0.5f};
    float m_sampleRate{48000.0f};
};

} // namespace Host

#endif // EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H
