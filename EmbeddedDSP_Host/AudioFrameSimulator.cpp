#include "AudioFrameSimulator.h"

#include <algorithm>
#include <cmath>
#include <numbers>

AudioFrameSimulator::AudioFrameSimulator(QObject *parent)
    : QObject(parent)
{
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &AudioFrameSimulator::onTick);
    setIntervalMs(16); // ~60 UI updates/s; enough for spectrum, light on CPU
}

void AudioFrameSimulator::start()
{
    if (m_timer.isActive()) {
        return;
    }

    m_sequence = 0;
    m_phase = 0.0f;
    m_timer.start();
    emit runningChanged(true);
}

void AudioFrameSimulator::stop()
{
    if (!m_timer.isActive()) {
        return;
    }

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
    AudioFramePacket frame{};
    frame.sof = 0xA6;
    frame.sequenceNumber = m_sequence++;
    frame.payloadLength = static_cast<uint16_t>(AUDIO_PACKET_SAMPLES * sizeof(int16_t));
    frame.crc16 = 0;

    constexpr float twoPi = 2.0f * std::numbers::pi_v<float>;

    // Robimy pełny "Sweep" (syrenę), która wędruje od 1000 Hz do 22000 Hz
    // Pełny cykl góra-dół zamyka się co 300 ramek
    float sweepPhase = ((m_sequence % 300) / 300.0f) * twoPi;

    // Środek pasma ustawiamy na 11500 Hz, a amplituda wahania to 10500 Hz
    // Daje to zakres od 1000 Hz (Bin ~2.6) do 22000 Hz (Bin ~58)
    float currentFreq = 11500.0f + 10500.0f * std::sin(sweepPhase);

    // Krok fazy dla tonu głównego
    const float phaseStep1 = twoPi * currentFreq / m_sampleRate;

    // Dorzucamy delikatną harmoniczną (np. 1.2 raza wyższą), o ile nie wyleci poza 24kHz
    float harmonicFreq = currentFreq * 1.2f;
    if (harmonicFreq > 23500.0f) {
        harmonicFreq = 23500.0f; // ścinamy, żeby nie było aliasingu
    }
    const float phaseStep2 = twoPi * harmonicFreq / m_sampleRate;

    // Poziomy głośności (65% ton główny, 35% harmoniczna, bez szumu dla idealnego odczytu)
    const float scalePrimary = m_amplitude * 32767.0f * 0.65f;
    const float scaleHarmonic = m_amplitude * 32767.0f * 0.35f;

    float phase2 = m_phase * 1.2f;

    for (std::size_t i = 0; i < AUDIO_PACKET_SAMPLES; ++i) {
        float signal = (scalePrimary * std::sin(m_phase)) + (scaleHarmonic * std::sin(phase2));

        frame.samples[i] = static_cast<int16_t>(std::clamp(signal, -32768.0f, 32767.0f));

        m_phase += phaseStep1;
        if (m_phase >= twoPi) m_phase -= twoPi;

        phase2 += phaseStep2;
        if (phase2 >= twoPi) phase2 -= twoPi;
    }

    emit audioFrameReceived(frame);
}
