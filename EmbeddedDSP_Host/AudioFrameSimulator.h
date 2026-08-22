#ifndef EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H
#define EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H

#include <QObject>
#include <QTimer>
#include <cstdint>

#include "Protocol/AudioFramePacket.h"

/**
 * @brief Generates synthetic AudioFramePacket streams for host-side UI/FFT work
 *        without a connected MCU. Mirrors SerialManager's audioFrameReceived signal.
 */
class AudioFrameSimulator : public QObject
{
    Q_OBJECT

public:
    explicit AudioFrameSimulator(QObject *parent = nullptr);

    void start();
    void stop();
    [[nodiscard]] bool isRunning() const;

    void setFrequencyHz(float frequencyHz);
    void setAmplitude(float amplitudeNormalized); // 0.0 .. 1.0
    void setIntervalMs(int intervalMs);

    [[nodiscard]] float frequencyHz() const;
    [[nodiscard]] float amplitude() const;

signals:
    void audioFrameReceived(const AudioFramePacket &frame);
    void runningChanged(bool running);

private slots:
    void onTick();

private:
    QTimer m_timer;
    uint8_t m_sequence{0};
    float m_phase{0.0f};
    float m_frequencyHz{1000.0f};
    float m_amplitude{0.5f}; // of full-scale Q15
    float m_sampleRate{48000.0f};
};

#endif // EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H