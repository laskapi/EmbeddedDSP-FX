#ifndef EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
#define EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H

#include <QFrame>
#include <memory>

class QComboBox;
class QPushButton;
class QHBoxLayout;

namespace Host { class AppController; }

namespace Host::UI {

/// @brief Toolbar for managing device connections.
class ConnectionToolbar : public QFrame {
    Q_OBJECT
public:
    explicit ConnectionToolbar(std::shared_ptr<AppController> controller, QWidget *parent = nullptr);

private slots:
    void refreshPortList();
    void onConnectionButtonClicked();
    void setConnected(bool connected);

private:
    [[nodiscard]] QString selectedPortName() const;

    std::shared_ptr<AppController> m_appController;
    
    QHBoxLayout *m_layout{nullptr};
    QComboBox *m_portCombo{nullptr};
    QPushButton *m_connectButton{nullptr};

    bool m_isConnected{false};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
