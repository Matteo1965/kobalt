#pragma once
#include <atomic>
#include <memory>
#include <QtGlobal>

class QObject;
class QThread;

namespace kobalt {
// Shared queue for reader sessions and their coordinate probes. Separate
// documents must not access CREngine's global fonts and caches concurrently.
QObject *engineWorker();
// Queued user work must preempt future idle pagination steps. Calls from the
// engine worker itself are not external requests.
quint64 engineWorkRequests();
qint64 engineWorkQuietMilliseconds();
QObject *engineWorkerForIdle();

struct EngineWorker
{
    QThread *thread = nullptr;
    QObject *object = nullptr;
    std::atomic_bool stopped{ false };
};

// One worker serializes CREngine's process-global font state across closing
// sessions and new openings. The state outlives the worker so retained
// documents can still be released during application destruction.
const std::shared_ptr<EngineWorker> &engineWorkerState();
} // namespace kobalt
