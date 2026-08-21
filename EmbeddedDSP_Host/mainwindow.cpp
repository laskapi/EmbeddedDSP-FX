#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Scan available COM ports on startup
    qDebug() << "--- Available Serial Ports ---";
    const auto ports = QSerialPortInfo::availablePorts();

    if (ports.isEmpty()) {
        qDebug() << "No serial ports detected!";
    } else {
        for (const QSerialPortInfo &info : ports) {
            qDebug() << "Port:" << info.portName()
            << "| Description:" << info.description()
            << "| Manufacturer:" << info.manufacturer();
        }
    }
    qDebug() << "------------------------------";
}

MainWindow::~MainWindow()
{
    delete ui;
}