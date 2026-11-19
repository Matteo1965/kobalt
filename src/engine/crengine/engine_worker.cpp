#include "engine_worker.h"
#include "engine_activity.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>

namespace kobalt {
namespace {
std::atomic<quint64> workRequests{ 0 };
std::atomic<qint64> lastRequest{ 0 };

qint64 monotonicMilliseconds()
{
    static const QElapsedTimer clock = [] {
        QElapsedTimer timer;
        timer.start();
        return timer;
    }();
    return clock.elapsed();
}
} // namespace

const std::shared_ptr<EngineWorker> &engineWorkerState()
{
    static const auto state = [] {
        auto state = std::make_shared<EngineWorker>();
        auto *thread = state->thread = new QThread(qApp);
        auto *object = state->object = new QObject;
        object->moveToThread(thread);
        QObject::connect(thread, &QThread::finished, object, &QObject::deleteLater);
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, thread, [state] {
            engineActivity().pause(false);
            state->thread->quit();
            state->thread->wait();
            state->stopped.store(true);
        });
        thread->start();
        return state;
    }();
    return state;
}

QObject *engineWorker()
{
    const auto &state = engineWorkerState();
    if (QThread::currentThread() != state->thread) {
        workRequests.fetch_add(1);
        lastRequest.store(monotonicMilliseconds());
    }
    return state->object;
}

QObject *engineWorkerForIdle()
{
    return engineWorkerState()->object;
}

quint64 engineWorkRequests()
{
    return workRequests.load();
}

qint64 engineWorkQuietMilliseconds()
{
    return monotonicMilliseconds() - lastRequest.load();
}
} // namespace kobalt
