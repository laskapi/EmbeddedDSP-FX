#ifndef EMBEDDEDDSP_HOST_MAINWINDOW_H
#define EMBEDDEDDSP_HOST_MAINWINDOW_H

#include <QMainWindow>
#include "SerialManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui{nullptr};
    SerialManager m_serialManager;
};

#endif // EMBEDDEDDSP_HOST_MAINWINDOW_H