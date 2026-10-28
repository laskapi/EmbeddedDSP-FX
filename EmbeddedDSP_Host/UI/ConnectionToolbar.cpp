#include "UI/ConnectionToolbar.h"
#include "Core/AppController.h"
#include "Models/ConnectionInfo.h"
#include "Device/Demo/DemoConnectionConstants.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>

namespace {

QString connectionComboLabel(const Host::Models::ConnectionInfo& info)
{
    if (info.id == Host::Device::kDemoConnectionId) {
        return info.description.isEmpty() ? info.id : info.description;
    }
    if (info.description.isEmpty()) {
        return info.id;
    }
    return QStringLiteral("%1  (%2)").arg(info.id, info.description);
}

} // namespace

namespace Host::UI {

ConnectionToolbar::ConnectionToolbar(Core::AppController& controller, QWidget *parent)
    : QFrame(parent)
    , m_appController(controller)
{
    m_connectionCombo = new QComboBox(this);
    m_connectionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_connectButton = new QPushButton(QStringLiteral("Connect"), this);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->addWidget(m_connectionCombo);
    m_layout->addWidget(m_connectButton);
    m_layout->addStretch();

    connect(m_connectButton, &QPushButton::clicked, this, &ConnectionToolbar::onConnectionButtonClicked);
    connect(&m_appController, &Core::AppController::connectionsUpdated,
            this, &ConnectionToolbar::setConnections);
    connect(&m_appController, &Core::AppController::connectionChanged,
            this, &ConnectionToolbar::setConnected);
}

void ConnectionToolbar::setConnections(const std::vector<Host::Models::ConnectionInfo>& connections) {
    if (m_isConnected) {
        return;
    }

    const QString previousId = selectedConnectionId();

    m_connectionCombo->clear();
    auto* model = qobject_cast<QStandardItemModel*>(m_connectionCombo->model());
    if (!model) {
        return;
    }

    for (const auto& info : connections) {
        auto* item = new QStandardItem(connectionComboLabel(info));
        item->setData(info.id, Qt::UserRole);
        item->setEnabled(info.selectable);
        model->appendRow(item);
    }

    int idx = m_connectionCombo->findData(previousId);
    if (idx == -1 || !model->item(idx) || !model->item(idx)->isEnabled()) {
        idx = firstSelectableIndex();
    }
    if (idx != -1) {
        m_connectionCombo->setCurrentIndex(idx);
    }

    m_connectButton->setEnabled(firstSelectableIndex() != -1);
    m_connectionCombo->updateGeometry();
}

QString ConnectionToolbar::selectedConnectionId() const {
    return m_connectionCombo->currentData().toString();
}

bool ConnectionToolbar::isCurrentSelectionSelectable() const {
    const int idx = m_connectionCombo->currentIndex();
    if (idx < 0) {
        return false;
    }
    const auto* model = qobject_cast<const QStandardItemModel*>(m_connectionCombo->model());
    if (!model || !model->item(idx)) {
        return false;
    }
    return model->item(idx)->isEnabled();
}

int ConnectionToolbar::firstSelectableIndex() const {
    const auto* model = qobject_cast<const QStandardItemModel*>(m_connectionCombo->model());
    if (!model) {
        return -1;
    }
    for (int row = 0; row < model->rowCount(); ++row) {
        if (const QStandardItem* item = model->item(row); item && item->isEnabled()) {
            return row;
        }
    }
    return -1;
}

void ConnectionToolbar::setConnected(bool connected) {
    m_isConnected = connected;
    m_connectButton->setText(m_isConnected ? QStringLiteral("Disconnect")
                                           : QStringLiteral("Connect"));
    m_connectionCombo->setEnabled(!m_isConnected);
    if (!m_isConnected) {
        m_connectButton->setEnabled(firstSelectableIndex() != -1);
    } else {
        m_connectButton->setEnabled(true);
    }
}

void ConnectionToolbar::onConnectionButtonClicked() {
    if (m_isConnected) {
        m_appController.disconnectDevice();
        return;
    }

    if (!isCurrentSelectionSelectable()) {
        return;
    }

    const QString connectionId = selectedConnectionId();
    if (connectionId.isEmpty()) {
        return;
    }

    m_appController.connectDevice(connectionId);
}

} // namespace Host::UI
