#ifndef EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
#define EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H

#include <QWidget>

class QComboBox;
class QPushButton;
class QHBoxLayout;

namespace Host {
    class StateManager;
    namespace UI {

class ConnectionToolbar : public QWidget {
    Q_OBJECT

public:
    explicit ConnectionToolbar(StateManager* manager, bool demoEnabled = true, QWidget *parent = nullptr);

    void refreshPortList();
    [[nodiscard]] QString selectedPortName() const;

public slots:
    void setConnected(bool connected);
    void setDemoRunning(bool running);

private slots:
    void onConnectionButtonClicked();
    void onDemoButtonClicked();

private:
    StateManager* m_manager{nullptr};
    QHBoxLayout *m_layout{nullptr};
    QComboBox *m_portCombo{nullptr};
    QPushButton *m_connectButton{nullptr};
    QPushButton *m_demoButton{nullptr};
    bool m_demoEnabled{true};
    bool m_isConnected{false};
    bool m_isDemoRunning{false};
};

} // namespace UI
} // namespace Host

#endif // EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
