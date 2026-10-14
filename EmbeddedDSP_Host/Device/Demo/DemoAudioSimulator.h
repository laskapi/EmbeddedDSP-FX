#ifndef EMBEDDEDDSP_HOST_DEMOAUDIOSIMULATOR_H
#define EMBEDDEDDSP_HOST_DEMOAUDIOSIMULATOR_H

#include <QTimer>
#include <cstdint>
#include <Protocol/AudioFramePacket.h>

namespace Host::Device::Demo {

/// @brief Generates synthetic audio data for testing without hardware.
class DemoAudioSimulator : public QObject {
    Q_OBJECT
public:
    explicit DemoAudioSimulator(QObject* parent = nullptr);

public slots:
    void start();
    void stop();

signals:
    void audioFrameReady(const Protocol::AudioFramePacket& frame);

private slots:
    void onTick();

private:
    QTimer m_timer;
    uint8_t m_sequence{0};
    float m_phase{0.0f};
};

} // namespace Host::Device::Demo

#endif // EMBEDDEDDSP_HOST_DEMOAUDIOSIMULATOR_H
