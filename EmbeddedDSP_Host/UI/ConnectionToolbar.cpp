#include "ConnectionToolbar.h"
#include "AppController.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSerialPortInfo>
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

#ifdef HOST_SIMULATOR_ENABLED
    m_demoButton = new QPushButton(QStringLiteral("Demo"), this);
    m_layout->addWidget(m_demoButton);
    connect(m_demoButton, &QPushButton::clicked,
            this, &ConnectionToolbar::onDemoButtonClicked);
    connect(m_appController.get(), &AppController::demoRunningChanged, this, &ConnectionToolbar::setDemoRunning);

#endif

    m_layout->addStretch();

    connect(m_connectButton, &QPushButton::clicked,
            this, &ConnectionToolbar::onConnectionButtonClicked);

    QTimer *scanTimer = new QTimer(this);
    connect(scanTimer, &QTimer::timeout, this, &ConnectionToolbar::refreshPortList);
    scanTimer->start(2000);
    refreshPortList(); 
    connect(m_appController.get(), &AppController::connectionChanged, this, &ConnectionToolbar::setConnected);
  }

void ConnectionToolbar::refreshPortList() {
    QString currentPort = selectedPortName();
    
    m_portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &info : ports) {
        const QString label = QStringLiteral("%1  (%2)").arg(info.portName(), info.description());
        m_portCombo->addItem(label, info.portName());
    }
#ifdef HOST_SIMULATOR_ENABLED
    m_portCombo->addItem(QStringLiteral("Simulator (Virtual Port)"), QStringLiteral("Simulator"));
#endif
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

void ConnectionToolbar::setDemoRunning(bool running) {
    m_isDemoRunning = running;
    if (m_demoButton) {
        m_demoButton->setText(m_isDemoRunning ? QStringLiteral("Stop Demo")
                                              : QStringLiteral("Demo"));
    }
}

void ConnectionToolbar::onConnectionButtonClicked() {
    if (m_isConnected) {
        m_appController->disconnect();
    } else {
        m_appController->connect(selectedPortName());
    }
}

#ifdef HOST_SIMULATOR_ENABLED
void ConnectionToolbar::onDemoButtonClicked() {
    if (m_isDemoRunning) {
        m_appController->stopDemo();
    } else {
        m_appController->startDemo();
    }
}
#endif

} // namespace Host::UI
