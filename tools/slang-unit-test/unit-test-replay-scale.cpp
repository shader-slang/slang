// unit-test-replay-scale.cpp
//
// Soak / scale tests. The compile-time knobs default to CI-friendly sizes;
// override via -D for serious stress runs. These tests guard against
// pathological behavior (quadratic blow-ups, missed dedup, lock contention)
// that only shows up at scale and would otherwise lurk until a user records
// a real workload.
//
// This branch carries the concurrent-dedup test from this file, which maps
// to the #10479 thread-safety bullet (dedup arm).

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <atomic>
#include <thread>
#include <vector>

// Concurrent dedup: many threads recording the same payload still produce
// exactly one disk file.
SLANG_UNIT_TEST(replayContextScaleConcurrentBlobHashingDedup)
{
    // A regression (per-thread hash table, lost-update on the dedup map)
    // would produce 2-8 files instead of 1. Issue #10479 thread-safety
    // bullet, dedup arm.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Per-test directory so the file count we assert on isn't mixed with
    // other tests' output.
    ctx().setReplayDirectory(".slang-replays-scale-concurrent");
    ctx().setMode(Mode::Record);

    static const uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    const int threads = 8;
    const int perThread = 250;

    // Atomic success counter. Any thread exception or lost call would
    // leave `ok` short of the expected total.
    std::atomic<int> ok{0};

    // Each worker hammers recordBlobByHash with the same payload, taking
    // ctx().lock() per call. ReplayContext's direct record* methods are
    // designed to be called while the caller holds the context lock (the
    // proxy layer does this implicitly via RECORD_CALL); calling them
    // unlocked from multiple threads races on m_stream and on the on-disk
    // dedup state and corrupts the heap. Catches exceptions silently so a
    // thread crash doesn't take down the runner; the post-loop SLANG_CHECK
    // on `ok` is what surfaces any losses.
    auto worker = [&]()
    {
        try
        {
            for (int i = 0; i < perThread; ++i)
            {
                SLANG_UNUSED(i);
                Slang::ComPtr<ISlangBlob> b = Slang::RawBlob::create(payload, sizeof(payload));
                ISlangBlob* p = b.get();
                {
                    auto guard = ctx().lock();
                    ctx().recordBlobByHash(RecordFlag::Output, p);
                }
                ok++;
            }
        }
        catch (...)
        {
        }
    };

    // Spawn the threads and join them all before checking results.
    std::vector<std::thread> th;
    for (int t = 0; t < threads; ++t)
        th.emplace_back(worker);
    for (auto& x : th)
        x.join();

    // Every recordBlobByHash call must have completed.
    SLANG_CHECK(ok == threads * perThread);

    // Walk the on-disk files/ directory. With one shared payload across
    // every thread, dedup must collapse all 2000 calls down to one file.
    const char* replayPath = ctx().getCurrentReplayPath();
    SLANG_CHECK(replayPath != nullptr);
    Slang::String filesDir = Slang::Path::combine(Slang::String(replayPath), "files");

    int fileCount = 0;
    struct Counter : public Slang::Path::Visitor
    {
        int* count;
        void accept(Slang::Path::Type type, const Slang::UnownedStringSlice&) override
        {
            if (type == Slang::Path::Type::File)
                (*count)++;
        }
    } counter;
    counter.count = &fileCount;
    (void)Slang::Path::find(filesDir, nullptr, &counter);
    SLANG_CHECK(fileCount == 1);

    ctx().disable();
    ctx().reset();
    Slang::Path::removeNonEmpty(ctx().getReplayDirectory());
    ctx().setReplayDirectory(".slang-replays");
}
