#ifndef EMBEDDEDDSP_HOST_GET_REQUEST_DEBOUNCER_H
#define EMBEDDEDDSP_HOST_GET_REQUEST_DEBOUNCER_H

#include "Protocol/ControlPacket.h"
#include <QObject>

class QTimer;

namespace Host::Core {

/// @brief Coalesces SetParam / SetMasterVolume into delayed Get* packets.
class GetRequestDebouncer : public QObject {
    Q_OBJECT
public:
    static constexpr int kDebounceTimeoutMs = 250;

    explicit GetRequestDebouncer(QObject* parent = nullptr);

    void noteParamChanged(int slot, int paramId);
    void noteMasterChanged();
    void stop();

signals:
    void getParamPacketReady(Protocol::ControlPacket pkt);
    void getMasterPacketReady(Protocol::ControlPacket pkt);

private:
    void onParamTimeout();
    void onMasterTimeout();

    QTimer* m_paramTimer{nullptr};
    QTimer* m_masterTimer{nullptr};
    int m_slot{-1};
    int m_paramId{-1};
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_GET_REQUEST_DEBOUNCER_H
