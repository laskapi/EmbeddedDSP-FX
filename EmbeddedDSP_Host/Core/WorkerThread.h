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
    [[nodiscard]] bool isRunning() const;

    /// @brief Moves @p object onto this thread and deletes it when the thread finishes.
    void adopt(QObject* object);

    /// @brief Stops auto-delete on thread finish (call before manual deleteLater).
    void release(QObject* object);

    /// @brief True when @p object currently has affinity to this thread.
    [[nodiscard]] bool hasAffinity(const QObject* object) const;

private:
    QThread m_thread;
};

} // namespace Host::Core

#endif // EMBEDDEDDSP_HOST_WORKER_THREAD_H
