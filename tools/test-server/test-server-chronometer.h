// test-server-chronometer.h
// Lightweight chronometer for timing test-server phases
// Enabled via SLANG_TEST_DIAGNOSTICS=timing environment variable

#ifndef TEST_SERVER_CHRONOMETER_H
#define TEST_SERVER_CHRONOMETER_H

#include "../../source/core/slang-dictionary.h"
#include "../../source/core/slang-process.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-test-diagnostics.h"

#include <chrono>
#include <cstdio>
#include <cstring>

namespace TestServer
{
using namespace Slang;

// Timing information for a single event type
struct TimingInfo
{
    int invocationCount = 0;
    std::chrono::nanoseconds totalDuration = std::chrono::nanoseconds::zero();
    std::chrono::nanoseconds minDuration = std::chrono::nanoseconds::max();
    std::chrono::nanoseconds maxDuration = std::chrono::nanoseconds::zero();
};

// Simple chronometer for test-server phase timing
// Enabled via SLANG_TEST_DIAGNOSTICS=timing (or "all")
// Phase details enabled via SLANG_TEST_DIAGNOSTICS=timing-phases
class Chronometer
{
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::nanoseconds;

    // Start timing a test - returns start time for use with endTest
    // Returns invalid time point if timing is disabled, so executionTimeMs stays at default 0.0
    TimePoint beginTest()
    {
        if (!isEnabled())
            return TimePoint{};
        return Clock::now();
    }

    // End timing a test and return duration in milliseconds
    // Returns 0.0 if timing is disabled or start time is invalid
    double endTest(TimePoint startTime)
    {
        if (startTime == TimePoint{})
            return 0.0;

        auto endTime = Clock::now();
        auto duration = std::chrono::duration_cast<Duration>(endTime - startTime);
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000.0;
    }

    // Start timing a named event (returns invalid time if disabled)
    TimePoint start(const char* eventName)
    {
        if (!isEnabled())
            return TimePoint{};

        if (isPhaseLoggingEnabled())
        {
            fprintf(stderr, "[TIMING] %s started (pid=%u)\n", eventName, Process::getId());
        }
        return Clock::now();
    }

    // Stop timing and record the result
    void stop(const char* eventName, TimePoint startTime)
    {
        if (!isEnabled() || startTime == TimePoint{})
            return;

        auto endTime = Clock::now();
        auto duration = std::chrono::duration_cast<Duration>(endTime - startTime);

        // Find or create entry for this event
        TimingInfo* info = nullptr;
        for (auto& kv : m_data)
        {
            if (strcmp(kv.key, eventName) == 0)
            {
                info = &kv.value;
                break;
            }
        }
        if (!info)
        {
            m_data.add(eventName, TimingInfo{});
            // Get the newly added entry
            info = &m_data.getLast().value;
        }

        info->invocationCount++;
        info->totalDuration += duration;
        if (duration < info->minDuration)
            info->minDuration = duration;
        if (duration > info->maxDuration)
            info->maxDuration = duration;

        if (isPhaseLoggingEnabled())
        {
            auto ms =
                std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000.0;
            fprintf(
                stderr,
                "[TIMING] %s completed: %.3fms (pid=%u)\n",
                eventName,
                ms,
                Process::getId());
        }
    }

    // Print summary report to stderr (only when phase logging is enabled)
    void printSummary()
    {
        if (!isPhaseLoggingEnabled() || m_data.getCount() == 0)
            return;

        fprintf(
            stderr,
            "\n[TIMING-SUMMARY] Test Server Timing Report (pid=%u)\n",
            Process::getId());
        fprintf(
            stderr,
            "%-35s %8s %12s %12s %12s %12s\n",
            "Event",
            "Count",
            "Total(ms)",
            "Avg(ms)",
            "Min(ms)",
            "Max(ms)");
        fprintf(
            stderr,
            "%s\n",
            "--------------------------------------------------------------------------------------"
            "---------");

        for (const auto& kv : m_data)
        {
            auto totalMs =
                std::chrono::duration_cast<std::chrono::microseconds>(kv.value.totalDuration)
                    .count() /
                1000.0;
            auto avgMs = kv.value.invocationCount > 0 ? totalMs / kv.value.invocationCount : 0.0;
            auto minMs = std::chrono::duration_cast<std::chrono::microseconds>(kv.value.minDuration)
                             .count() /
                         1000.0;
            auto maxMs = std::chrono::duration_cast<std::chrono::microseconds>(kv.value.maxDuration)
                             .count() /
                         1000.0;

            // Handle the case where minDuration was never updated
            if (kv.value.invocationCount == 0)
            {
                minMs = 0.0;
            }

            fprintf(
                stderr,
                "%-35s %8d %12.3f %12.3f %12.3f %12.3f\n",
                kv.key,
                kv.value.invocationCount,
                totalMs,
                avgMs,
                minMs,
                maxMs);
        }
        fprintf(stderr, "\n");
    }

    // Check if timing is enabled (SLANG_TEST_DIAGNOSTICS=timing)
    static bool isEnabled()
    {
        static bool enabled = isDiagnosticEnabled("timing");
        return enabled;
    }

    // Check if phase-level logging is enabled (SLANG_TEST_DIAGNOSTICS=timing-phases)
    static bool isPhaseLoggingEnabled()
    {
        static bool enabled = isDiagnosticEnabled("timing-phases");
        return enabled;
    }

    // Clear all timing data
    void clear() { m_data.clear(); }

private:
    OrderedDictionary<const char*, TimingInfo> m_data;
};

// RAII helper for automatic timing of scopes
class ScopedTimer
{
public:
    ScopedTimer(Chronometer& chrono, const char* eventName)
        : m_chronometer(chrono), m_eventName(eventName), m_startTime(chrono.start(eventName))
    {
    }

    ~ScopedTimer() { m_chronometer.stop(m_eventName, m_startTime); }

    // Non-copyable
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    Chronometer& m_chronometer;
    const char* m_eventName;
    Chronometer::TimePoint m_startTime;
};

// Convenience macro for timing a scope
// Usage: SCOPED_TIMER(m_chronometer, "phase-name");
#define SCOPED_TIMER(chrono, name) ::TestServer::ScopedTimer _scopedTimer##__LINE__(chrono, name)

} // namespace TestServer

#endif // TEST_SERVER_CHRONOMETER_H
