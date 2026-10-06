#include "ConnectionToolbar.h"
#include "AppController.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>

namespace Host::UI {

ConnectionToolbar::ConnectionToolbar(std::shared_ptr<AppController> controller, QWidget *parent)
    : QFrame(parent)
    , m_appController(controller) {
    Q_ASSERT(m_appController);
    
    m_portCombo = new QComboBox(this);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), this);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->addWidget(m_portCombo);
    m_layout->addWidget(m_connectButton);
    m_layout->addStretch();

    connect(m_connectButton, &QPushButton::clicked, this, &ConnectionToolbar::onConnectionButtonClicked);
    connect(m_appController.get(), &AppController::connectionChanged, this, &ConnectionToolbar::setConnected);

    QTimer *scanTimer = new QTimer(this);
    connect(scanTimer, &QTimer::timeout, this, &ConnectionToolbar::refreshPortList);
    scanTimer->start(2000);
    
    refreshPortList(); 
}

void ConnectionToolbar::refreshPortList() {
    QString currentPort = selectedPortName();
    
    m_portCombo->clear();
    const auto connections = m_appController->availableConnections();
    
    for (const auto &info : connections) {
        const QString label = info.description.isEmpty() ? info.id : 
                              QStringLiteral("%1  (%2)").arg(info.id, info.description);
        m_portCombo->addItem(label, info.id);
    }

    int idx = m_portCombo->findData(currentPort);
    if (idx != -1) m_portCombo->setCurrentIndex(idx);
    
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
        m_appController->disconnect();
    } else {
        m_appController->connectToDevice(selectedPortName());
    }
}

} // namespace Host::UI
