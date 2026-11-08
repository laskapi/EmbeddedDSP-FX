#ifndef EMBEDDEDDSP_FIRMWARE_USB_TX_H
#define EMBEDDEDDSP_FIRMWARE_USB_TX_H

#include "Comm/ManifestBuilder.h"
#include "DSP/FxChain.h"
#include "SharedBuffer.h"
#include "Protocol/AudioFrame.h"
#include "Protocol/ControlPacket.h"
#include "Protocol/ProtocolCommon.h"
#include "SpscQueue.h"
#include <cassert>
#include <optional>

#ifndef FIRMWARE_UNIT_TESTS
#include "usbd_cdc_if.h"
#else
/// @brief Unit-test stub for USB CDC transport (always accepts).
inline uint8_t CDC_Transmit_FS(uint8_t*, uint16_t) { return 0; }
#endif

namespace App::Comm {

/// @brief Single owner of USB CDC TX (control sync + audio frames).
///
/// GetState: ManifestFollows → manifest bytes (+ '\\0') → Report* packets (TX priority).
/// After sync (Idle): pending Report* drain before audio frames.
class UsbTx {
public:
    using TransmitFn = uint8_t (*)(uint8_t* buf, uint16_t len);

    explicit UsbTx(TransmitFn transmit = &CDC_Transmit_FS) noexcept
        : m_transmit(transmit) {}

    /// @brief Starts GetState reply and enables host audio streaming.
    void beginSync(App::Dsp::FxChain& chain) noexcept {
        if (m_phase != Phase::Idle) {
            return;
        }
        m_audioStreamEnabled = true;
        clearReports();
        using enum App::SharedBuffer::Owner;
        assert(App::g_sharedBuffer.acquire(Manifest));
        const auto manifestBuf = App::g_sharedBuffer.get<char>(Manifest);
        m_manifestSize = writeManifest(manifestBuf, App::Dsp::FxChain::kMaxEffectSlots);
        enqueueAllReports(chain);
        m_phase = Phase::Marker;
    }

    [[nodiscard]] bool isSyncActive() const noexcept { return m_phase != Phase::Idle; }

    [[nodiscard]] bool isAudioStreamEnabled() const noexcept { return m_audioStreamEnabled; }

    void setAudioStreamEnabled(bool enabled) noexcept { m_audioStreamEnabled = enabled; }

    /// @brief Queues ReportEffectType + Bypass + Params for one slot (Idle only).
    void enqueueSlotReports(App::Dsp::FxChain& chain, uint8_t slotId) noexcept {
        if (isSyncActive() || slotId >= App::Dsp::FxChain::kMaxEffectSlots) {
            return;
        }
        enqueueSlotReportsUnchecked(chain, slotId);
    }

    /// @brief Queues ReportBypass for one slot (Idle only).
    void enqueueBypassReport(uint8_t slotId, bool bypassed) noexcept {
        if (isSyncActive() || slotId >= App::Dsp::FxChain::kMaxEffectSlots) {
            return;
        }
        pushReport(Protocol::Command::ReportBypass, slotId, 0, bypassed ? 1.0f : 0.0f);
    }

    /// @brief Queues ReportMasterVolume (Idle only).
    void enqueueMasterVolumeReport(float gain) noexcept {
        if (isSyncActive()) {
            return;
        }
        pushReport(Protocol::Command::ReportMasterVolume, 0, 0, gain);
    }

    /// @brief Zero-copy audio slot; drops oldest frame if the ring is full.
    [[nodiscard]] Protocol::AudioFrame& acquireAudioFrame() noexcept {
        Protocol::AudioFrame* slot = m_audioTx.acquireWrite();
        return *slot;
    }

    /// @brief Stamps SOF/sequence/CRC and publishes frame from acquireAudioFrame.
    void commitAudioFrame(Protocol::AudioFrame& frame) noexcept {
        frame.sof = Protocol::SOF::Audio;
        frame.sequence = m_audioSequence++;
        frame.applyCRC();
        m_audioTx.publishWrite();
    }

    /// @brief Sends one USB payload (retries m_pending if CDC busy).
    void sendNext() noexcept {
        // Retry path: do not arm anything new while a previous TX is still pending.
        if (m_pending) {
            flushPending(m_pending);
            return;
        }

        if (m_phase != Phase::Idle) {
            switch (m_phase) {
                case Phase::Marker: {
                    m_txPacket = {};
                    m_txPacket.command = Protocol::Command::ManifestFollows;
                    m_txPacket.applyCRC();
                    m_pending = Pending::from(&m_txPacket, sizeof(m_txPacket));
                    flushPending(m_pending);
                    break;
                }
                case Phase::Manifest: {
                    const auto arena = App::g_sharedBuffer.get(App::SharedBuffer::Owner::Manifest);
                    m_pending = Pending::from(arena.data, m_manifestSize + 1); // + '\0'
                    flushPending(m_pending);
                    break;
                }
                case Phase::Reports: {
                    if (tryArmNextReport()) {
                        flushPending(m_pending);
                    } else {
                        finishSync();
                    }
                    break;
                }
                default:
                    break;
            }
            return;
        }

        // Idle: async Report* before audio (control wins over stream).
        if (tryArmNextReport()) {
            flushPending(m_pending);
            return;
        }

        if (!m_audioStreamEnabled) {
            return;
        }

        if (auto frame = m_audioTx.pop()) {
            m_txFrame = *frame;
            m_pending = Pending::from(&m_txFrame, sizeof(m_txFrame));
            flushPending(m_pending);
        }
    }

private:
    enum class Phase : uint8_t { Idle, Marker, Manifest, Reports };

    /// @brief Armed TX: points at m_txPacket, m_txFrame, or SharedBuffer (Manifest).
    struct Pending {
        const uint8_t* ptr{nullptr};
        uint16_t length{0};

        template<typename T>
        [[nodiscard]] static Pending from(const T* data, std::size_t length) noexcept {
            assert(length <= 0xFFFFu);
            return Pending{reinterpret_cast<const uint8_t*>(data),
                           static_cast<uint16_t>(length)};
        }
    };

    static constexpr std::size_t kAudioTxCapacity = 4;
    static constexpr std::size_t kReportCapacity = 32;

    /// @brief CDC TX of @p pending (must be set); busy leaves it, OK clears and advances sync phase.
    void flushPending(std::optional<Pending>& pending) noexcept {
        assert(pending.has_value());
        if (m_transmit(const_cast<uint8_t*>(pending->ptr), pending->length) != 0) {
            return; // busy — keep pending for next sendNext
        }
        pending.reset();
        if (m_phase == Phase::Marker) {
            m_phase = Phase::Manifest;
        } else if (m_phase == Phase::Manifest) {
            m_phase = Phase::Reports;
        }
    }

    void finishSync() noexcept {
        m_phase = Phase::Idle;
        m_manifestSize = 0;
        App::g_sharedBuffer.release(App::SharedBuffer::Owner::Manifest);
    }

    void clearReports() noexcept {
        while (m_reports.pop()) {
        }
    }

    [[nodiscard]] bool tryArmNextReport() noexcept {
        if (auto pkt = m_reports.pop()) {
            m_txPacket = *pkt;
            m_pending = Pending::from(&m_txPacket, sizeof(m_txPacket));
            return true;
        }
        return false;
    }

    void pushReport(Protocol::Command cmd, uint8_t slot, uint8_t paramOrType, float val) noexcept {
        Protocol::ControlPacket pkt{};
        pkt.command = cmd;
        pkt.slotId = slot;
        pkt.paramId = paramOrType;
        pkt.setValue(val);
        pkt.applyCRC();
        (void)m_reports.push(pkt);
    }

    void enqueueSlotReportsUnchecked(App::Dsp::FxChain& chain, uint8_t slotId) noexcept {
        std::visit([&](auto& fx) {
            using T = std::decay_t<decltype(fx)>;
            pushReport(Protocol::Command::ReportEffectType, slotId, static_cast<uint8_t>(T::Id), 0.0f);
            pushReport(Protocol::Command::ReportBypass, slotId, 0, fx.isBypassed() ? 1.0f : 0.0f);
            for (uint8_t p = 0; p < T::ParamCount; ++p) {
                pushReport(Protocol::Command::ReportParam, slotId, p, fx.paramValue(p));
            }
        }, chain.slot(slotId));
    }

    void enqueueAllReports(App::Dsp::FxChain& chain) noexcept {
        for (uint8_t slotId = 0; slotId < App::Dsp::FxChain::kMaxEffectSlots; ++slotId) {
            enqueueSlotReportsUnchecked(chain, slotId);
        }
        pushReport(Protocol::Command::ReportMasterVolume, 0, 0, chain.masterGain());
    }

    TransmitFn m_transmit;
    Phase m_phase{Phase::Idle};
    bool m_audioStreamEnabled{false};
    std::size_t m_manifestSize{0};
    std::uint8_t m_audioSequence{0};
    App::SpscQueue<Protocol::ControlPacket, kReportCapacity> m_reports{};
    App::SpscQueue<Protocol::AudioFrame, kAudioTxCapacity> m_audioTx{};
    Protocol::ControlPacket m_txPacket{};
    Protocol::AudioFrame m_txFrame{};
    std::optional<Pending> m_pending{};
};

} // namespace App::Comm

#endif // EMBEDDEDDSP_FIRMWARE_USB_TX_H
