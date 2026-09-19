#ifndef EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H
#define EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H

#include <QObject>
#include <QTimer>
#include <cstdint>
#include <Protocol/AudioFramePacket.h>

/**
 * @brief Generates synthetic AudioFramePacket streams for host-side UI/FFT testing.
 * 
 * Used when no physical MCU is connected.
 */
class AudioFrameSimulator : public QObject
{
    Q_OBJECT

public:
    explicit AudioFrameSimulator(QObject *parent = nullptr);

    /** @brief Starts the simulation timer. */
    void start();
    
    /** @brief Stops the simulation timer. */
    void stop();
    
    /** @return True if simulation is running. */
    [[nodiscard]] bool isRunning() const;

    /** @brief Configures the test sine wave frequency. */
    void setFrequencyHz(float frequencyHz);
    
    /** @brief Configures the test sine wave amplitude (0.0 to 1.0). */
    void setAmplitude(float amplitudeNormalized);
    
    /** @brief Configures the transmission interval in milliseconds. */
    void setIntervalMs(int intervalMs);

    [[nodiscard]] float frequencyHz() const;
    [[nodiscard]] float amplitude() const;

signals:
    /** @brief Emitted when a new simulated frame is ready. */
    void audioFrameReceived(const Protocol::AudioFramePacket &frame);
    
    /** @brief Emitted when simulator state changes. */
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

#endif // EMBEDDEDDSP_HOST_AUDIOFRAMESIMULATOR_H
