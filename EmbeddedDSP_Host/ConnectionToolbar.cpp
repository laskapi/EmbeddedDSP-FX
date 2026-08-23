#include "ConnectionToolbar.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSerialPortInfo>

ConnectionToolbar::ConnectionToolbar(bool demoEnabled, QWidget *parent)
    : QWidget(parent)
    , m_demoEnabled(demoEnabled)
{
    m_portCombo = new QComboBox(this);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), this);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->addWidget(m_portCombo);
    m_layout->addWidget(m_connectButton);

    if (m_demoEnabled) {
        m_demoButton = new QPushButton(QStringLiteral("Demo"), this);
        m_layout->addWidget(m_demoButton);
        connect(m_demoButton, &QPushButton::clicked,
                this, &ConnectionToolbar::onDemoButtonClicked);
    }

    m_layout->addStretch();

    connect(m_connectButton, &QPushButton::clicked,
            this, &ConnectionToolbar::onConnectionButtonClicked);

    refreshPortList();
}

void ConnectionToolbar::refreshPortList()
{
    m_portCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        const QString label = QStringLiteral("%1  (%2)")
        .arg(info.portName(), info.description());
        m_portCombo->addItem(label, info.portName());
    }

    m_connectButton->setEnabled(m_portCombo->count() > 0);
}

QString ConnectionToolbar::selectedPortName() const
{
    return m_portCombo->currentData().toString();
}

void ConnectionToolbar::setConnected(bool connected)
{
    m_connectButton->setText(connected ? QStringLiteral("Disconnect")
                                       : QStringLiteral("Connect"));
    m_portCombo->setEnabled(!connected);
}

void ConnectionToolbar::setDemoRunning(bool running)
{
    if (m_demoButton) {
        m_demoButton->setText(running ? QStringLiteral("Stop Demo")
                                      : QStringLiteral("Demo"));
    }
}

void ConnectionToolbar::onConnectionButtonClicked()
{
    if (m_connectButton->text() == QStringLiteral("Disconnect")) {
        emit disconnectRequested();
        return;
    }

    emit connectRequested(selectedPortName());
}

void ConnectionToolbar::onDemoButtonClicked()
{
    if (m_demoButton && m_demoButton->text() == QStringLiteral("Stop Demo")) {
        emit demoStopRequested();
        return;
    }

    emit demoStartRequested();
}