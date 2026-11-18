#include "Core/GetRequestDebouncer.h"

#include <QTimer>
#include <QtGlobal>

namespace Host::Core {

namespace {

[[nodiscard]] uint8_t toProtocolByte(int value) {
    Q_ASSERT(value >= 0 && value <= 255);
    return static_cast<uint8_t>(value);
}

} // namespace

GetRequestDebouncer::GetRequestDebouncer(QObject* parent)
    : QObject(parent)
    , m_paramTimer(new QTimer(this))
    , m_masterTimer(new QTimer(this))
{
    m_paramTimer->setSingleShot(true);
    connect(m_paramTimer, &QTimer::timeout, this, &GetRequestDebouncer::onParamTimeout);

    m_masterTimer->setSingleShot(true);
    connect(m_masterTimer, &QTimer::timeout, this, &GetRequestDebouncer::onMasterTimeout);
}

void GetRequestDebouncer::noteParamChanged(int slot, int paramId) {
    m_slot = slot;
    m_paramId = paramId;
    m_paramTimer->start(kDebounceTimeoutMs);
}

void GetRequestDebouncer::noteMasterChanged() {
    m_masterTimer->start(kDebounceTimeoutMs);
}

void GetRequestDebouncer::stop() {
    m_paramTimer->stop();
    m_masterTimer->stop();
    m_slot = -1;
    m_paramId = -1;
}

void GetRequestDebouncer::onParamTimeout() {
    if (m_slot < 0 || m_paramId < 0) {
        return;
    }

    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::GetParam;
    pkt.slotId = toProtocolByte(m_slot);
    pkt.paramId = toProtocolByte(m_paramId);
    pkt.setValue(0.0f);
    emit getParamPacketReady(pkt);
}

void GetRequestDebouncer::onMasterTimeout() {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::GetMasterVolume;
    pkt.setValue(0.0f);
    emit getMasterPacketReady(pkt);
}

} // namespace Host::Core
