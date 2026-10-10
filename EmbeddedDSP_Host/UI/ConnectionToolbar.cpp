#include "ConnectionToolbar.h"
#include "AppController.h"
#include "ConnectionInfo.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>

namespace Host::UI {

ConnectionToolbar::ConnectionToolbar(Core::AppController& controller, QWidget *parent)
    : QFrame(parent)
    , m_appController(controller)
{
    m_portCombo = new QComboBox(this);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), this);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->addWidget(m_portCombo);
    m_layout->addWidget(m_connectButton);
    m_layout->addStretch();

    connect(m_connectButton, &QPushButton::clicked, this, &ConnectionToolbar::onConnectionButtonClicked);
    connect(&m_appController, &Core::AppController::connectionsUpdated,
            this, &ConnectionToolbar::setConnections);
    connect(&m_appController, &Core::AppController::connectionChanged,
            this, [this](bool connected, const QString&) {
                setConnected(connected);
            });
}

void ConnectionToolbar::setConnections(const std::vector<Host::Models::ConnectionInfo>& connections) {
    if (m_isConnected) {
        return;
    }

    const QString currentPort = selectedPortName();

    m_portCombo->clear();
    for (const auto& info : connections) {
        const QString label = info.description.isEmpty()
                                  ? info.id
                                  : QStringLiteral("%1  (%2)").arg(info.id, info.description);
        m_portCombo->addItem(label, info.id);
    }

    const int idx = m_portCombo->findData(currentPort);
    if (idx != -1) {
        m_portCombo->setCurrentIndex(idx);
    }

    m_connectButton->setEnabled(m_portCombo->count() > 0);
}

QString ConnectionToolbar::selectedPortName() const {
    return m_portCombo->currentData().toString();
}

void ConnectionToolbar::setConnected(bool connected) {
    m_isConnected = connected;
    m_connectButton->setText(m_isConnected ? QStringLiteral("Disconnect")
                                           : QStringLiteral("Connect"));
    m_portCombo->setEnabled(!m_isConnected);
}

void ConnectionToolbar::onConnectionButtonClicked() {
    if (m_isConnected) {
        m_appController.disconnectFromDevice();
    } else {
        m_appController.connectToDevice(selectedPortName());
    }
}

} // namespace Host::UI
