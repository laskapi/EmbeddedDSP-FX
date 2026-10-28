#include "Core/AppController.h"
#include "UI/mainwindow.h"
#include "Models/ConnectionInfo.h"
#include "Device/ConnectionScanner.h"
#include "Device/DeviceFactory.h"
#include "Device/DeviceSession.h"
#include "Processing/AudioFrameWorker.h"

#include <QApplication>
#include <QFile>
#include "Protocol/ControlPacket.h"
#include "Protocol/AudioFramePacket.h"
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

    auto connectionScanner = std::make_unique<Host::Device::ConnectionScanner>();
    auto deviceSession = std::make_unique<Host::Device::DeviceSession>(
        std::make_unique<Host::Device::DeviceFactory>());
    auto audioWorker = std::make_unique<Host::Processing::AudioFrameWorker>();

    auto appController = std::make_unique<Host::Core::AppController>(
        std::move(connectionScanner), std::move(deviceSession), std::move(audioWorker));

    Host::UI::MainWindow w(*appController);
    w.show();
    return QApplication::exec();
}
