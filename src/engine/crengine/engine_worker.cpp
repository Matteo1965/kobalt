#include "engine_worker.h"
#include "engine_activity.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QTimer>
#include <functional>

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

namespace {
struct IdleStepTask
{
    std::function<bool()> step;
    quint64 requestGeneration = 0;
    qint64 minimumQuiet = 0;
};

// A continuation is queued only after the previous short step returns. It
// never blocks the GUI waiting for a quiet period, and normal engine events
// are processed before the next idle step.
void queueIdleAttempt(const std::shared_ptr<IdleStepTask> &task, int delay = 0)
{
    QTimer::singleShot(delay, engineWorkerForIdle(), [task] {
        if (engineWorkRequests() != task->requestGeneration)
            return;
        const qint64 quiet = engineWorkQuietMilliseconds();
        if (quiet < task->minimumQuiet) {
            const auto remaining = task->minimumQuiet - quiet;
            queueIdleAttempt(task, int(qMin<qint64>(remaining, 100)));
            return;
        }
        // Every step must yield promptly; a long, uninterruptible render
        // would still block user requests and is not suitable for this API.
        if (task->step())
            queueIdleAttempt(task);
    });
}
} // namespace

void scheduleEngineIdleSteps(std::function<bool()> step, qint64 minimumQuietMilliseconds)
{
    if (!step)
        return;
    auto task = std::make_shared<IdleStepTask>();
    task->step = std::move(step);
    task->requestGeneration = engineWorkRequests();
    task->minimumQuiet = qMax<qint64>(0, minimumQuietMilliseconds);
    queueIdleAttempt(task);
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
