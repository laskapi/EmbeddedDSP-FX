#include "AudioFrameSimulator.h"

#include <algorithm>
#include <cmath>
#include <numbers>

AudioFrameSimulator::AudioFrameSimulator(QObject *parent)
    : QObject(parent)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &AudioFrameSimulator::onTick);
    setIntervalMs(16);
}

void AudioFrameSimulator::start()
{
    if (m_timer.isActive()) return;
    m_sequence = 0;
    m_phase = 0.0f;
    m_timer.start();
    emit runningChanged(true);
}

void AudioFrameSimulator::stop()
{
    if (!m_timer.isActive()) return;
    m_timer.stop();
    emit runningChanged(false);
}

bool AudioFrameSimulator::isRunning() const
{
    return m_timer.isActive();
}

void AudioFrameSimulator::setFrequencyHz(float frequencyHz)
{
    m_frequencyHz = std::clamp(frequencyHz, 20.0f, m_sampleRate * 0.45f);
}

void AudioFrameSimulator::setAmplitude(float amplitudeNormalized)
{
    m_amplitude = std::clamp(amplitudeNormalized, 0.0f, 1.0f);
}

void AudioFrameSimulator::setIntervalMs(int intervalMs)
{
    m_timer.setInterval(std::clamp(intervalMs, 1, 1000));
}

float AudioFrameSimulator::frequencyHz() const
{
    return m_frequencyHz;
}

float AudioFrameSimulator::amplitude() const
{
    return m_amplitude;
}

void AudioFrameSimulator::onTick()
{
    Protocol::AudioFramePacket frame{};
    frame.sequenceNumber = m_sequence++;
    frame.applyCRC();

    constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    // Sweep from 1000 Hz to 22000 Hz
    float sweepPhase = ((m_sequence % 300) / 300.0f) * twoPi;
    float currentFreq = 11500.0f + 10500.0f * std::sin(sweepPhase);

    const float phaseStep1 = twoPi * currentFreq / m_sampleRate;

    // Harmonic at 1.2x freq
    float harmonicFreq = currentFreq * 1.2f;
    if (harmonicFreq > 23500.0f) harmonicFreq = 23500.0f;
    const float phaseStep2 = twoPi * harmonicFreq / m_sampleRate;

    const float scalePrimary = m_amplitude * 32767.0f * 0.65f;
    const float scaleHarmonic = m_amplitude * 32767.0f * 0.35f;

    float phase2 = m_phase * 1.2f;

    for (std::size_t i = 0; i < Protocol::AUDIO_SAMPLES; ++i) {
        float signal = (scalePrimary * std::sin(m_phase)) + (scaleHarmonic * std::sin(phase2));
        frame.samples[i] = static_cast<int16_t>(std::clamp(signal, -32768.0f, 32767.0f));

        m_phase += phaseStep1;
        if (m_phase >= twoPi) m_phase -= twoPi;
        phase2 += phaseStep2;
        if (phase2 >= twoPi) phase2 -= twoPi;
    }

    emit audioFrameReceived(frame);
}
