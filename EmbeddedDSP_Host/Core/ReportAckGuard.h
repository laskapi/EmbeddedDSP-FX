#ifndef EMBEDDEDDSP_HOST_REPORT_ACK_GUARD_H
#define EMBEDDEDDSP_HOST_REPORT_ACK_GUARD_H

#include <QObject>
#include <cstdint>

class QTimer;

namespace Host::Core {

/// @brief Waits for Report* after host commands; soft GetState then hard reconnect on timeout.
/// Inactive until start(slotCount); safe no-op Null Object while stopped.
class ReportAckGuard : public QObject {
    Q_OBJECT
public:
    static constexpr int kAckTimeoutMs = 1000;

    explicit ReportAckGuard(QObject* parent = nullptr);

    void start(int slotCount);
    void stop();

    [[nodiscard]] bool isActive() const { return m_slotCount > 0; }

    void reset();

    void expectEffectType(int slot);
    void expectEffectTypesAll();
    void expectBypass(int slot);
    void expectParam(int slot, int paramId);
    void expectMasterVolume();

    void onReportEffectType(int slot);
    void onReportBypass(int slot);
    void onReportParam(int slot, int paramId);
    void onReportMasterVolume();

signals:
    void recoverSoft();
    void recoverHard();

private:
    enum class Phase { Idle, WaitReports, WaitSync };

    [[nodiscard]] uint8_t slotBit(int slot) const;
    void armAck();
    void onAckTimeout();
    [[nodiscard]] bool hasPending() const;
    void clearPending();

    QTimer* m_ackTimer{nullptr};
    Phase m_phase{Phase::Idle};
    int m_slotCount{0};
    uint8_t m_pendingTypes{0};
    uint8_t m_pendingBypass{0};
    bool m_pendingMaster{false};
    bool m_pendingParam{false};
    int m_paramSlot{-1};
    int m_paramId{-1};
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_REPORT_ACK_GUARD_H
