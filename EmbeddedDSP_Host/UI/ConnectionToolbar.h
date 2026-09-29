#ifndef EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
#define EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H

#include <memory>
#include <QFrame>

class QComboBox;
class QPushButton;
class QHBoxLayout;

namespace Host { class AppController; }

namespace Host::UI {

class ConnectionToolbar : public QFrame {
    Q_OBJECT

public:
    explicit ConnectionToolbar(std::shared_ptr<AppController> controller, bool demoEnabled = true, QWidget *parent = nullptr);

    void refreshPortList();
    [[nodiscard]] QString selectedPortName() const;

public slots:
    void setConnected(bool connected);
    void setDemoRunning(bool running);

private slots:
    void onConnectionButtonClicked();
    void onDemoButtonClicked();

private:
    std::shared_ptr<AppController> m_appController;
    QHBoxLayout *m_layout{nullptr};
    QComboBox *m_portCombo{nullptr};
    QPushButton *m_connectButton{nullptr};
    QPushButton *m_demoButton{nullptr};
    bool m_demoEnabled{true};
    bool m_isConnected{false};
    bool m_isDemoRunning{false};
};

} // namespace Host::UI

#endif // EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
