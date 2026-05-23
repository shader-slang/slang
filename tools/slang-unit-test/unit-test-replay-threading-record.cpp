// unit-test-replay-threading-record.cpp
//
// Thread-safety tests for the recording side of the replay system. Each test
// drives multiple threads through the public Slang API (the proxies under
// the hood share a single ReplayContext mutex) and then validates that
// every call ran successfully and that the resulting stream contains data.
//
// Coverage scope: these tests are smoke checks for gross lock omissions and
// for the "all threads complete; stream is non-empty" contract. At 4-8
// threads and 1-25 calls each, a missing per-call lock can still serialize
// by scheduler luck on a CI runner and pass undetected. The default sizes
// favor fast CI feedback; real race detection comes from running the
// process under TSAN, or from cranking the stress test up via -D (see
// SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_* below).

#include "unit-test-replay-common.h"

#include <atomic>
#include <thread>
#include <vector>

// CI-friendly defaults. Crank up via -D for actual concurrency stress testing.
#ifndef SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_THREAD_COUNT
#define SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_THREAD_COUNT 8
#endif
#ifndef SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_CALLS_PER_THREAD
#define SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_CALLS_PER_THREAD 25
#endif

// =============================================================================
// Concurrent recording: multiple threads each create sessions simultaneously
// =============================================================================

// Concurrent createSession() through one shared global-session proxy must
// produce a valid stream.
SLANG_UNIT_TEST(replayContextConcurrentRecording)
{
    // A regression that lost the per-call lock would surface as a torn
    // stream, dropped calls, or assertion failures inside the proxy
    // internals. This is the simplest form of that test; the API-call and
    // stress variants below add depth.
    REPLAY_TEST;

    ctx().setMode(Mode::Record);

    // Build the shared global session up front, single-threaded, so the
    // concurrent phase only contends on createSession() rather than also
    // racing on the global-session bootstrap.
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    const int numThreads = 4;

    // Atomic counters drive the post-join checks: one increment per thread
    // for either branch, so successCount + failCount must equal numThreads.
    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};

    // Each thread creates exactly one session. Catching `...` ensures a
    // thread-local exception doesn't propagate out of std::thread (which
    // would call std::terminate).
    auto threadFunc = [&](int)
    {
        try
        {
            slang::SessionDesc sessionDesc = {};
            Slang::ComPtr<slang::ISession> session;
            SlangResult result = globalSession->createSession(sessionDesc, session.writeRef());
            if (SLANG_SUCCEEDED(result) && session != nullptr)
                successCount++;
            else
                failCount++;
        }
        catch (...)
        {
            failCount++;
        }
    };

    // Spawn and join. Reserving the vector avoids a reallocation in the
    // emplace loop that could invalidate the thread handles.
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc, i);

    for (auto& t : threads)
        t.join();

    // Every thread must have succeeded and the stream must contain bytes
    // from at least one of them.
    SLANG_CHECK(successCount == numThreads);
    SLANG_CHECK(failCount == 0);
    SLANG_CHECK(ctx().getStream().getSize() > 0);
}
// =============================================================================
// Concurrent API calls: multiple threads call findProfile on a shared proxy
// =============================================================================

// Tighter contention check: 4 threads x 10 findProfile calls each.
SLANG_UNIT_TEST(replayContextConcurrentApiCalls)
{
    // findProfile is a read-only API, so recorded entries are all one
    // shape. This is essentially a contention check on the per-context
    // lock under steady-state recording load.
    REPLAY_TEST;

    ctx().setMode(Mode::Record);

    // Bootstrap the shared global session single-threaded.
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    const int numThreads = 4;
    const int callsPerThread = 10;
    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};

    // Each thread issues callsPerThread findProfile calls. Any exception
    // counts as one failure rather than aborting the whole thread, so the
    // post-join totals tell us how many actual losses we saw.
    auto threadFunc = [&](int)
    {
        try
        {
            for (int j = 0; j < callsPerThread; j++)
            {
                SlangProfileID profile = globalSession->findProfile("sm_5_0");
                if (profile != SLANG_PROFILE_UNKNOWN)
                    successCount++;
                else
                    failCount++;
            }
        }
        catch (...)
        {
            failCount++;
        }
    };

    // Spawn and join.
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc, i);

    for (auto& t : threads)
        t.join();

    // All numThreads*callsPerThread findProfile calls must have succeeded.
    SLANG_CHECK(successCount == numThreads * callsPerThread);
    SLANG_CHECK(failCount == 0);
    SLANG_CHECK(ctx().getStream().getSize() > 0);
}
// =============================================================================

// Tunable stress companion to ConcurrentApiCalls.
SLANG_UNIT_TEST(replayContextConcurrentApiCallsStress)
{
    // Same shape as ConcurrentApiCalls, but the thread count and calls per
    // thread come from SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_THREAD_COUNT
    // and SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_CALLS_PER_THREAD. Defaults
    // are CI-friendly; override at compile time for serious stress runs.
    REPLAY_TEST;

    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    const int numThreads = SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_THREAD_COUNT;
    const int callsPerThread = SLANG_UNIT_TEST_REPLAY_THREAD_STRESS_CALLS_PER_THREAD;
    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};

    auto threadFunc = [&](int)
    {
        try
        {
            for (int j = 0; j < callsPerThread; j++)
            {
                SlangProfileID profile = globalSession->findProfile("sm_5_0");
                if (profile != SLANG_PROFILE_UNKNOWN)
                    successCount++;
                else
                    failCount++;
            }
        }
        catch (...)
        {
            failCount++;
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(size_t(numThreads));
    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc, i);
    for (auto& t : threads)
        t.join();

    SLANG_CHECK(successCount == numThreads * callsPerThread);
    SLANG_CHECK(failCount == 0);
    SLANG_CHECK(ctx().getStream().getSize() > 0);
}
