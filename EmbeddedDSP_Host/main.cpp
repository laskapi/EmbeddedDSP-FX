#include "AppController.h"
#include "mainwindow.h"
#include "ConnectionInfo.h"
#include "ConnectionScanner.h"
#include "DeviceFactory.h"
#include "AudioFrameWorker.h"

#include <QApplication>
#include <QFile>
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <memory>
#include <vector>

int main(int argc, char* argv[]) {
    qRegisterMetaType<Protocol::ControlPacket>("Protocol::ControlPacket");
    qRegisterMetaType<Protocol::AudioFramePacket>("Protocol::AudioFramePacket");
    qRegisterMetaType<std::vector<float>>("std::vector<float>");
    qRegisterMetaType<std::vector<Host::Models::ConnectionInfo>>(
        "std::vector<Host::Models::ConnectionInfo>");

    QApplication a(argc, argv);

    QFile styleFile(":/UI/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
    }

    auto scanner = std::make_unique<Host::Device::ConnectionScanner>();
    auto factory = std::make_unique<Host::Device::DeviceFactory>();
    auto worker = std::make_unique<Host::Processing::AudioFrameWorker>();

    auto appController = std::make_unique<Host::Core::AppController>(
        std::move(scanner), std::move(factory), std::move(worker));

    Host::UI::MainWindow w(*appController);
    w.show();
    return QApplication::exec();
}
