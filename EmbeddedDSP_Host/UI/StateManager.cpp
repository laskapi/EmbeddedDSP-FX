#include "StateManager.h"
#include <QStringList>
#include <QDebug>

namespace UI {

StateManager::StateManager(QObject* parent) : QObject(parent) {
    connect(&m_serial, &SerialManager::manifestReceived, this, &StateManager::onManifestReceived);
    connect(&m_serial, &SerialManager::controlPacketReceived, this, &StateManager::onControlPacketReceived);
    connect(&m_serial, &SerialManager::portStatusChanged, this, &StateManager::connectionChanged);
}

void StateManager::setParameter(uint8_t slotId, uint8_t paramId, float value) {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetParam;
    pkt.slotId = slotId;
    pkt.paramId = paramId;
    pkt.setValue(value);
    pkt.applyCRC();
    m_serial.sendControlPacket(pkt);
}

void StateManager::setEffectType(uint8_t slotId, uint8_t effectTypeId) {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::SetEffectType;
    pkt.slotId = slotId;
    pkt.effectTypeId = effectTypeId;
    pkt.applyCRC();
    m_serial.sendControlPacket(pkt);
}

void StateManager::setBypass(uint8_t slotId, bool bypassed) {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::BypassToggle;
    pkt.slotId = slotId;
    pkt.setValue(bypassed ? 1.0f : 0.0f);
    pkt.applyCRC();
    m_serial.sendControlPacket(pkt);
}

void StateManager::requestSync() {
    Protocol::ControlPacket pkt;
    pkt.command = Protocol::Command::GetState;
    pkt.applyCRC();
    m_serial.sendControlPacket(pkt);
}

void StateManager::onManifestReceived(const QString& manifest) {
    parseManifest(manifest);
    emit manifestProcessed();
}

void StateManager::onControlPacketReceived(const Protocol::ControlPacket& pkt) {
    emit deviceStateUpdated(pkt);
}

void StateManager::parseManifest(const QString& manifest) {
    m_catalog.clear();
    const QStringList effectLines = manifest.split('\n', Qt::SkipEmptyParts);

    for (const QString &effectLine : effectLines) {
        QStringList segments = effectLine.split(';', Qt::SkipEmptyParts);
        if (segments.isEmpty()) continue;

        QStringList header = segments[0].split(':');
        if (header.isEmpty()) continue;

        const QString tag = header[0];

        if (tag == QLatin1String("C")) {
            if (header.size() >= 3 && header[1] == QLatin1String("SLOTS")) {
                m_slotCount = header[2].toInt();
            }
        } else if (tag == QLatin1String("E")) {
            if (header.size() < 3) continue;

            Host::EffectSpec spec;
            spec.id = static_cast<uint8_t>(header[1].toInt());
            spec.name = header[2];

            auto params = segments.sliced(1);
            for (const QString &param : params) {
                QStringList p = param.split(':');
                if (p.size() < 5 || p[0] != QLatin1String("P")) continue;
                
                spec.params.append({p[1], p[2].toFloat(), p[3].toFloat(), p[4].toFloat()});
            }
            m_catalog[spec.id] = spec;
        }
    }

    if (m_slotCount <= 0) m_slotCount = 4;
}

} // namespace UI
