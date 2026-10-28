#include "Core/WorkerThread.h"
#include <QObject>

namespace Host::Core {

WorkerThread::~WorkerThread()
{
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait();
    }
}

void WorkerThread::start()
{
    m_thread.start();
}

void WorkerThread::adopt(QObject* object)
{
    object->moveToThread(&m_thread);
    QObject::connect(&m_thread, &QThread::finished, object, &QObject::deleteLater);
}

} // namespace Host::Core
