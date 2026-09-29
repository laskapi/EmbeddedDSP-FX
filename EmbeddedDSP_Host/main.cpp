#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <vector>

int main(int argc, char *argv[])
{
    qRegisterMetaType<Protocol::ControlPacket>("Protocol::ControlPacket");
    qRegisterMetaType<Protocol::AudioFramePacket>("Protocol::AudioFramePacket");
    qRegisterMetaType<std::vector<float>>("std::vector<float>");

    QApplication a(argc, argv);

    // Load stylesheet from embedded resources (compiled via qt_add_resources in CMake)
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
    }

    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
