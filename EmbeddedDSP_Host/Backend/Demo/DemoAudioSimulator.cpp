#include "DemoAudioSimulator.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Host::Backend::Demo {

namespace {
constexpr float DEFAULT_AMPLITUDE = 0.5f;
constexpr float SAMPLE_RATE = 48000.0f;
}

DemoAudioSimulator::DemoAudioSimulator(QObject *parent)
    : QObject(parent), m_timer(this)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &DemoAudioSimulator::onTick);
    m_timer.setInterval(16);
}

void DemoAudioSimulator::start()
{
    if (m_timer.isActive()) return;
    m_sequence = 0;
    m_phase = 0.0f;
    m_timer.start();
}

void DemoAudioSimulator::stop()
{
    if (!m_timer.isActive()) return;
    m_timer.stop();
}

bool DemoAudioSimulator::isRunning() const
{
    return m_timer.isActive();
}

void DemoAudioSimulator::onTick()
{
    Protocol::AudioFramePacket frame{};
    frame.sequence = m_sequence++;
    frame.applyCRC();

    constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    // Sweep from 1000 Hz to 22000 Hz
    float sweepPhase = ((m_sequence % 300) / 300.0f) * twoPi;
    float currentFreq = 11500.0f + 10500.0f * std::sin(sweepPhase);

    const float phaseStep1 = twoPi * currentFreq / SAMPLE_RATE;

    // Harmonic at 1.2x freq
    float harmonicFreq = currentFreq * 1.2f;
    if (harmonicFreq > 23500.0f) harmonicFreq = 23500.0f;
    const float phaseStep2 = twoPi * harmonicFreq / SAMPLE_RATE;

    const float scalePrimary = DEFAULT_AMPLITUDE * 32767.0f * 0.65f;
    const float scaleHarmonic = DEFAULT_AMPLITUDE * 32767.0f * 0.35f;

    float phase2 = m_phase * 1.2f;

    for (std::size_t i = 0; i < Protocol::AUDIO_SAMPLES; ++i) {
        float signal = (scalePrimary * std::sin(m_phase)) + (scaleHarmonic * std::sin(phase2));
        frame.samples[i] = static_cast<int16_t>(std::clamp(signal, -32768.0f, 32767.0f));

        m_phase += phaseStep1;
        if (m_phase >= twoPi) m_phase -= twoPi;
        phase2 += phaseStep2;
        if (phase2 >= twoPi) phase2 -= twoPi;
    }

    emit audioFrameReady(frame);
}

} // namespace Host::Backend::Demo
