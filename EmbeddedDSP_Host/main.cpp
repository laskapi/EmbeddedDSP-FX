#include "mainwindow.h"
#include <QApplication>
#include <Protocol/ControlPacket.h>
#include <Protocol/AudioFramePacket.h>
#include <vector>

int main(int argc, char *argv[])
{
    qRegisterMetaType<Protocol::ControlPacket>("Protocol::ControlPacket");
    qRegisterMetaType<Protocol::AudioFramePacket>("Protocol::AudioFramePacket");
    qRegisterMetaType<std::vector<float>>("std::vector<float>");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QCoreApplication::exec();
}
