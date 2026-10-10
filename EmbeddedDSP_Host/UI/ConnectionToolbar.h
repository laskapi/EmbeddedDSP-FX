#ifndef EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
#define EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H

#include <QFrame>
#include <vector>

class QComboBox;
class QPushButton;
class QHBoxLayout;

namespace Host::Core { class AppController; }
namespace Host::Models { struct ConnectionInfo; }

namespace Host::UI {

/// @brief Toolbar for managing device connections.
class ConnectionToolbar : public QFrame {
    Q_OBJECT
public:
    explicit ConnectionToolbar(Core::AppController& controller, QWidget *parent = nullptr);

private slots:
    void onConnectionButtonClicked();
    void setConnections(const std::vector<Host::Models::ConnectionInfo>& connections);
    void setConnected(bool connected);

private:
    [[nodiscard]] QString selectedPortName() const;

    Core::AppController& m_appController;
    QHBoxLayout *m_layout{nullptr};
    QComboBox *m_portCombo{nullptr};
    QPushButton *m_connectButton{nullptr};

    bool m_isConnected{false};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
