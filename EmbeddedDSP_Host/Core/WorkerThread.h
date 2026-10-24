#ifndef EMBEDDEDDSP_HOST_WORKER_THREAD_H
#define EMBEDDEDDSP_HOST_WORKER_THREAD_H

#include <QThread>

class QObject;

namespace Host::Core {

/// @brief Owns a QThread: start/stop and move QObjects onto it safely.
class WorkerThread {
public:
    WorkerThread() = default;
    ~WorkerThread();

    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;

    void start();

    /// @brief Moves @p object onto this thread and deletes it when the thread finishes.
    void adopt(QObject* object);

private:
    QThread m_thread;
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_WORKER_THREAD_H
