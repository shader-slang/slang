// unit-test-replay-scale.cpp
//
// Soak / scale tests. The compile-time knobs default to CI-friendly sizes;
// override via -D for serious stress runs. These tests guard against
// pathological behavior (quadratic blow-ups, missed dedup, lock contention)
// that only shows up at scale and would otherwise lurk until a user records
// a real workload.
//
// The dedup test here covers the lock-held form of the contract: many
// threads calling recordBlobByHash while each holds ctx().lock() must still
// collapse to one file. It is not a race-on-the-dedup-map test, since the
// per-call lock serializes execution (see in-body comment).

#include "../../source/core/slang-io.h"
#include "unit-test-replay-common.h"

#include <atomic>
#include <thread>
#include <vector>

// Concurrent dedup: many threads recording the same payload still produce
// exactly one disk file.
SLANG_UNIT_TEST(replayContextScaleConcurrentBlobHashingDedup)
{
    // A regression that broke dedup for lock-held callers (per-thread hash
    // table, lost-update on the dedup map under sequential calls) would
    // produce 2-8 files instead of 1. The per-call ctx().lock() below means
    // the threads are serialized through recordBlobByHash, so this is a
    // contract check on the lock-held path rather than a race-on-the-map
    // test; the value is breadth (multiple threads independently take and
    // release the recursive mutex) plus on-disk verification of dedup
    // output.
    REPLAY_TEST;

    // RAII: the scratch directory and the replay-directory setting are
    // restored on scope exit so a throw mid-test can't leak either into
    // later tests. The dedup file-count assertion below depends on having
    // a private directory, so this isolation matters in both directions.
    ScopedReplayDirectorySetting restoreReplayDir;
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

    // Best-effort: remove the scratch directory before the guard runs so the
    // file count we assert on doesn't linger past the test. The guard still
    // restores the default replay-directory setting on scope exit.
    ctx().disable();
    ctx().reset();
    Slang::Path::removeNonEmpty(ctx().getReplayDirectory());
}
