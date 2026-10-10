#include "AppController.h"
#include "mainwindow.h"
#include "ConnectionInfo.h"
#include "IDeviceBackend.h"
#ifdef HOST_SIMULATOR_ENABLED
#include "DemoBackend.h"
#else
#include "SerialBackend.h"
#endif

#include <QApplication>
#include <QFile>
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <memory>
#include <vector>

int main(int argc, char *argv[])
{
    qRegisterMetaType<Protocol::ControlPacket>("Protocol::ControlPacket");
    qRegisterMetaType<Protocol::AudioFramePacket>("Protocol::AudioFramePacket");
    qRegisterMetaType<std::vector<float>>("std::vector<float>");
    qRegisterMetaType<std::vector<Host::Models::ConnectionInfo>>(
        "std::vector<Host::Models::ConnectionInfo>");

    QApplication a(argc, argv);

    // Load stylesheet from embedded resources (compiled via qt_add_resources in CMake)
    QFile styleFile(":/UI/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
    }

    std::unique_ptr<Host::Backend::IDeviceBackend> backend;
#ifdef HOST_SIMULATOR_ENABLED
    backend = std::make_unique<Host::Backend::DemoBackend>();
#else
    backend = std::make_unique<Host::Backend::SerialBackend>();
#endif

    auto appController = std::make_unique<Host::Core::AppController>(std::move(backend));

    Host::UI::MainWindow w(*appController);
    w.show();
    return QCoreApplication::exec();
}
