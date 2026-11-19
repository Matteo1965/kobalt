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

// A cooperative checkpoint for future hybrid pagination. Background jobs
// capture the external-request generation before starting a short work step.
// They must yield when a user request arrives (even if it is already handled).
// No application or engine layout behavior changes until callers opt in.
class EngineIdleCheckpoint
{
public:
    explicit EngineIdleCheckpoint(qint64 minimumQuietMilliseconds = 0)
        : requestGeneration_(engineWorkRequests()), minimumQuiet_(minimumQuietMilliseconds)
    {
    }

    bool interrupted() const { return engineWorkRequests() != requestGeneration_; }

    bool canContinue() const
    {
        return !interrupted() && engineWorkQuietMilliseconds() >= minimumQuiet_;
    }

private:
    quint64 requestGeneration_;
    qint64 minimumQuiet_;
};

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
