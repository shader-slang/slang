// unit-test-replay-threading.cpp
//
// Thread safety tests for the replay system.
// Exercises concurrent recording through the ReplayContext mutex.

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
// Concurrent record → playback: N threads simultaneously call createSession on
// a shared global session in Record mode, then we replay the resulting stream
// in single-threaded playback and verify it consumes cleanly to EOS.
// =============================================================================
SLANG_UNIT_TEST(replayContextConcurrentRecordingThenPlayback)
{
    REPLAY_TEST;

    // --- Record phase setup: one shared global session, recorded ---
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    const int numThreads = 4;
    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};

    // Each worker thread independently creates one session on the shared
    // global session. The replay context's recursive_mutex (acquired by
    // RECORD_CALL) is what serialises the writes to the stream.
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

    // Launch all worker threads and join.
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(threadFunc, i);
    for (auto& t : threads)
        t.join();

    // Every thread should have succeeded — no fails, no exceptions.
    SLANG_CHECK(successCount == numThreads);
    SLANG_CHECK(failCount == 0);
    // The stream should hold the recorded calls.
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // --- Playback phase ---
    // Replay the concurrently-recorded stream. The interleaved-but-serialised
    // call records should consume cleanly all the way to EOS.
    ctx().switchToPlayback();
    ctx().executeAll();
    SLANG_CHECK(ctx().getStream().atEnd());
    ctx().disable();
}

// =============================================================================
// Mixed workload: half the threads create sessions, half query profiles. Two
// different recorded operations interleaving in the same stream stresses the
// fixed-overhead-per-call path more than the homogeneous test above.
// =============================================================================
SLANG_UNIT_TEST(replayContextConcurrentMixedWorkload)
{
    REPLAY_TEST;

    // --- Record phase setup ---
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.apiVersion = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, globalSession.writeRef())));

    const int numThreads = 8;
    const int sessionsPerThread = 5;
    const int profilesPerThread = 10;
    std::atomic<int> sessionCreates{0};
    std::atomic<int> profileQueries{0};
    std::atomic<int> failCount{0};

    // Worker A: create N sessions per thread.
    auto sessionThread = [&]()
    {
        try
        {
            for (int k = 0; k < sessionsPerThread; k++)
            {
                slang::SessionDesc sessionDesc = {};
                Slang::ComPtr<slang::ISession> session;
                if (SLANG_SUCCEEDED(
                        globalSession->createSession(sessionDesc, session.writeRef())) &&
                    session != nullptr)
                    sessionCreates++;
                else
                    failCount++;
            }
        }
        catch (...)
        {
            failCount++;
        }
    };

    // Worker B: query a profile name N times per thread. This intentionally
    // mixes with worker A so the stream contains interleaved record types.
    auto profileThread = [&]()
    {
        try
        {
            for (int k = 0; k < profilesPerThread; k++)
            {
                if (globalSession->findProfile("sm_5_0") != SLANG_PROFILE_UNKNOWN)
                    profileQueries++;
                else
                    failCount++;
            }
        }
        catch (...)
        {
            failCount++;
        }
    };

    // Launch half the threads as A, half as B, then join all.
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads / 2; i++)
        threads.emplace_back(sessionThread);
    for (int i = 0; i < numThreads / 2; i++)
        threads.emplace_back(profileThread);
    for (auto& t : threads)
        t.join();

    // Every thread should have done exactly its allotted work — no losses to
    // races, exceptions, or missed calls.
    SLANG_CHECK(sessionCreates == (numThreads / 2) * sessionsPerThread);
    SLANG_CHECK(profileQueries == (numThreads / 2) * profilesPerThread);
    SLANG_CHECK(failCount == 0);
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // --- Playback phase ---
    // Replay the heterogeneous, interleaved stream and confirm it consumes
    // to EOS. Any drift in record-vs-replay framing would surface here as an
    // exception or a non-empty residual stream.
    ctx().switchToPlayback();
    ctx().executeAll();
    SLANG_CHECK(ctx().getStream().atEnd());
    ctx().disable();
}
