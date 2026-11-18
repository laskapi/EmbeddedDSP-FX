#include "Core/ReportAckGuard.h"

#include <QTimer>

namespace Host::Core {

ReportAckGuard::ReportAckGuard(QObject* parent)
    : QObject(parent)
    , m_ackTimer(new QTimer(this))
{
    m_ackTimer->setSingleShot(true);
    connect(m_ackTimer, &QTimer::timeout, this, &ReportAckGuard::onAckTimeout);
}

void ReportAckGuard::start(int slotCount) {
    reset();
    // uint8_t bitmask supports at most 8 slots.
    if (slotCount < 1 || slotCount > 8) {
        m_slotCount = 0;
        return;
    }
    m_slotCount = slotCount;
}

void ReportAckGuard::stop() {
    reset();
    m_slotCount = 0;
}

uint8_t ReportAckGuard::slotBit(int slot) const {
    if (slot < 0 || slot >= m_slotCount) {
        return 0;
    }
    return static_cast<uint8_t>(1u << slot);
}

void ReportAckGuard::reset() {
    m_phase = Phase::Idle;
    clearPending();
    m_ackTimer->stop();
}

void ReportAckGuard::expectEffectType(int slot) {
    m_pendingTypes |= slotBit(slot);
    armAck();
}

void ReportAckGuard::expectEffectTypesAll() {
    m_pendingTypes = static_cast<uint8_t>((1u << m_slotCount) - 1u);
    armAck();
}

void ReportAckGuard::expectBypass(int slot) {
    m_pendingBypass |= slotBit(slot);
    armAck();
}

void ReportAckGuard::expectParam(int slot, int paramId) {
    m_pendingParam = true;
    m_paramSlot = slot;
    m_paramId = paramId;
    armAck();
}

void ReportAckGuard::expectMasterVolume() {
    m_pendingMaster = true;
    armAck();
}

void ReportAckGuard::onReportEffectType(int slot) {
    m_pendingTypes = static_cast<uint8_t>(m_pendingTypes & ~slotBit(slot));
    if (m_phase != Phase::Idle && !hasPending()) {
        reset();
    }
}

void ReportAckGuard::onReportBypass(int slot) {
    m_pendingBypass = static_cast<uint8_t>(m_pendingBypass & ~slotBit(slot));
    if (m_phase != Phase::Idle && !hasPending()) {
        reset();
    }
}

void ReportAckGuard::onReportParam(int slot, int paramId) {
    if (m_pendingParam && slot == m_paramSlot && paramId == m_paramId) {
        m_pendingParam = false;
        m_paramSlot = -1;
        m_paramId = -1;
    }
    if (m_phase != Phase::Idle && !hasPending()) {
        reset();
    }
}

void ReportAckGuard::onReportMasterVolume() {
    m_pendingMaster = false;
    if (m_phase != Phase::Idle && !hasPending()) {
        reset();
    }
}

void ReportAckGuard::armAck() {
    if (!isActive()) {
        return;
    }
    m_phase = Phase::WaitReports;
    m_ackTimer->start(kAckTimeoutMs);
}

void ReportAckGuard::onAckTimeout() {
    if (!isActive()) {
        return;
    }
    if (m_phase == Phase::WaitReports) {
        m_phase = Phase::WaitSync;
        clearPending();
        emit recoverSoft();
        m_ackTimer->start(kAckTimeoutMs);
        return;
    }
    if (m_phase == Phase::WaitSync) {
        reset();
        emit recoverHard();
    }
}

bool ReportAckGuard::hasPending() const {
    return m_pendingTypes != 0
        || m_pendingBypass != 0
        || m_pendingMaster
        || m_pendingParam;
}

void ReportAckGuard::clearPending() {
    m_pendingTypes = 0;
    m_pendingBypass = 0;
    m_pendingMaster = false;
    m_pendingParam = false;
    m_paramSlot = -1;
    m_paramId = -1;
}

} // namespace Host::Core
