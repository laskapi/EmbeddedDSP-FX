#ifndef EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H
#define EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H

#include <QWidget>

class QComboBox;
class QPushButton;
class QHBoxLayout;

/**
 * @brief Top toolbar: COM port selection, Connect/Disconnect, optional Demo toggle.
 *        UI only — emits user intents; does not own SerialManager or simulators.
 */
class ConnectionToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionToolbar(bool demoEnabled = true, QWidget *parent = nullptr);

    void refreshPortList();
    [[nodiscard]] QString selectedPortName() const;

public slots:
    void setConnected(bool connected);
    void setDemoRunning(bool running);

signals:
    void connectRequested(const QString &portName);
    void disconnectRequested();
    void demoStartRequested();
    void demoStopRequested();

private slots:
    void onConnectionButtonClicked();
    void onDemoButtonClicked();

private:
    QHBoxLayout *m_layout{nullptr};
    QComboBox *m_portCombo{nullptr};
    QPushButton *m_connectButton{nullptr};
    QPushButton *m_demoButton{nullptr};
    bool m_demoEnabled{true};
};

#endif // EMBEDDEDDSP_HOST_CONNECTIONTOOLBAR_H