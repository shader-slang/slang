#include "core/slang-render-api-util.h"
#include "render-test/slang-support.h"
#include "render-test/slang-test-device-cache.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>

namespace
{
void emitError(renderer_test::CoreToRHIDebugBridge& bridge, const char* message)
{
    bridge.handleMessage(rhi::DebugMessageType::Error, rhi::DebugMessageSource::Layer, message);
}

renderer_test::CoreToRHIDebugBridge* getRetainedBridgeAfterStackCallbackScope()
{
    auto bridge = renderer_test::createRetainedCoreToRHIDebugBridge();

    renderer_test::CoreDebugCallback callback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(*bridge, &callback);
        emitError(*bridge, "retained scope");
    }
    SLANG_CHECK(callback.getString() == "retained scope\n");

    return bridge.Ptr();
}
} // namespace

SLANG_UNIT_TEST(scopedCoreDebugCallbackClearsBridgeOnExit)
{
    renderer_test::CoreToRHIDebugBridge bridge;

    {
        renderer_test::CoreDebugCallback callback;
        {
            renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &callback);
            emitError(bridge, "inside scope");
            SLANG_CHECK(callback.getString() == "inside scope\n");
        }

        emitError(bridge, "after scope");
        SLANG_CHECK(callback.getString() == "inside scope\n");
    }

    emitError(bridge, "after callback");

    renderer_test::CoreDebugCallback nextCallback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &nextCallback);
        emitError(bridge, "next scope");
    }
    SLANG_CHECK(nextCallback.getString() == "next scope\n");
}

SLANG_UNIT_TEST(scopedCoreDebugCallbackDoesNotLeakAcrossScopes)
{
    renderer_test::CoreToRHIDebugBridge bridge;
    renderer_test::CoreDebugCallback firstCallback;
    renderer_test::CoreDebugCallback secondCallback;

    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &firstCallback);
        emitError(bridge, "first");
    }

    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &secondCallback);
        emitError(bridge, "second");
    }

    SLANG_CHECK(firstCallback.getString() == "first\n");
    SLANG_CHECK(secondCallback.getString() == "second\n");
}

SLANG_UNIT_TEST(scopedCoreDebugCallbackClearsBridgeOnException)
{
    renderer_test::CoreToRHIDebugBridge bridge;
    renderer_test::CoreDebugCallback firstCallback;

    try
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &firstCallback);
        emitError(bridge, "before throw");
        throw 1;
    }
    catch (...)
    {
    }

    emitError(bridge, "after exception");

    renderer_test::CoreDebugCallback secondCallback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &secondCallback);
        emitError(bridge, "next iteration");
    }

    SLANG_CHECK(firstCallback.getString() == "before throw\n");
    SLANG_CHECK(secondCallback.getString() == "next iteration\n");
}

SLANG_UNIT_TEST(scopedCoreDebugCallbackSeparatesRetainedBridgeScopes)
{
    renderer_test::CoreToRHIDebugBridge* oldBridge = getRetainedBridgeAfterStackCallbackScope();
    auto nextBridge = renderer_test::createRetainedCoreToRHIDebugBridge();

    renderer_test::CoreDebugCallback nextCallback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(*nextBridge, &nextCallback);
        emitError(*oldBridge, "after stack callback");
        emitError(*nextBridge, "next invocation");
    }
    SLANG_CHECK(nextCallback.getString() == "next invocation\n");
}

// Pins the contract that getString() returns storage independent of the bridge's
// internal buffer, deterministically and without needing a sanitizer.
//
// This is the single-threaded safety net for the data race fixed alongside it: the
// concurrent stress test below only fails under ThreadSanitizer, because the racy
// aliasing path leaves the old shared buffer immutable via copy-on-write, so a
// revert of getString() to the buffer-sharing `m_buf.toString()` would still pass
// every ordinary unit-test run. Here, two back-to-back snapshots taken with no
// intervening append must each own a distinct StringRepresentation; under the
// aliasing bug both would share the bridge's representation and compare equal.
SLANG_UNIT_TEST(coreDebugCallbackGetStringReturnsIndependentStorage)
{
    renderer_test::CoreToRHIDebugBridge bridge;
    renderer_test::CoreDebugCallback callback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &callback);
        emitError(bridge, "x");
    }

    Slang::String first = callback.getString();
    Slang::String second = callback.getString();

    // Same captured content...
    SLANG_CHECK(first == "x\n");
    SLANG_CHECK(first == second);
    // ...but backed by independent storage, not the bridge's shared buffer.
    SLANG_CHECK(first.getStringRepresentation() != second.getStringRepresentation());
}

// Pins the #11856 fix without a GPU: drives the CPU backend through DeviceCache::acquireDevice and
// checks a same-key hit returns the same device and bridge, a distinct key a distinct bridge, and a
// post-cleanCache re-acquire a fresh bridge. Before the fix a reused device kept a stale (cleared)
// bridge and dropped its validation messages. The emitError(*bridge, ...) calls below model a
// device-forwarded message (real device->debugCallback forwarding needs a GPU).
SLANG_UNIT_TEST(deviceCacheReusesDebugBridgeAcrossInvocations)
{
    using namespace renderer_test;

    // Skip when the CPU backend is not enabled for this run (e.g. a narrowed -api set) so the test
    // never hard-fails on an unrelated configuration.
    if ((unitTestContext->enabledApis & Slang::RenderApiFlag::CPU) == 0)
    {
        SLANG_IGNORE_TEST
    }

    // Start from a clean cache so the assertions are deterministic regardless of prior tests or a
    // harness retry that re-runs this module in-process.
    DeviceCache::cleanCache();

    // A descriptor whose cache key is unique to this test, so it cannot collide with another cached
    // device in the process.
    rhi::DeviceDesc desc = {};
    desc.deviceType = rhi::DeviceType::CPU;
    desc.slang.slangGlobalSession = unitTestContext->slangGlobalSession;
    desc.slang.targetProfile = "device-cache-bridge-regression-11856";

    // Invocation A: create the device and get the bridge it is wired to.
    Slang::ComPtr<rhi::IDevice> deviceA;
    Slang::RefPtr<CoreToRHIDebugBridge> bridgeA;
    if (SLANG_FAILED(DeviceCache::acquireDevice(desc, deviceA.writeRef(), &bridgeA)) || !deviceA)
    {
        // CPU device unavailable here - skip. (A successful acquire always yields a bridge, so the
        // check below is a real guard.)
        SLANG_IGNORE_TEST
    }
    SLANG_CHECK(bridgeA != nullptr);

    CoreDebugCallback callbackA;
    {
        ScopedCoreDebugCallback scopedA(*bridgeA, &callbackA);
        // Stand in for a validation message the device forwards through its bridge.
        emitError(*bridgeA, "invocation A");
    }
    SLANG_CHECK(callbackA.getString() == "invocation A\n");

    // Between invocations the bridge has no active callback, so a late device message is dropped
    // rather than written to dead storage (the #11785 contract).
    emitError(*bridgeA, "between invocations");

    // Invocation B: the same key is a cache hit, so the same device comes back wired to the same
    // bridge — the crux of the #11856 fix.
    Slang::ComPtr<rhi::IDevice> deviceB;
    Slang::RefPtr<CoreToRHIDebugBridge> bridgeB;
    SLANG_CHECK(SLANG_SUCCEEDED(DeviceCache::acquireDevice(desc, deviceB.writeRef(), &bridgeB)));
    SLANG_CHECK(deviceB.get() == deviceA.get()); // cache hit: same device
    SLANG_CHECK(bridgeB == bridgeA);             // and the bridge that device is actually wired to

    CoreDebugCallback callbackB;
    {
        ScopedCoreDebugCallback scopedB(*bridgeB, &callbackB);
        emitError(*bridgeA, "invocation B validation");
    }
    SLANG_CHECK(callbackB.getString() == "invocation B validation\n");

    // The dropped between-invocations message reached neither callback.
    SLANG_CHECK(callbackA.getString() == "invocation A\n");

    // A distinct key must get a distinct device and therefore a distinct bridge, so one config's
    // device never routes through another's callback (the per-key isolation the fix preserves).
    rhi::DeviceDesc descOther = desc;
    descOther.slang.targetProfile = "device-cache-bridge-regression-11856-other";
    Slang::ComPtr<rhi::IDevice> deviceOther;
    Slang::RefPtr<CoreToRHIDebugBridge> bridgeOther;
    SlangResult otherResult =
        DeviceCache::acquireDevice(descOther, deviceOther.writeRef(), &bridgeOther);
    if (SLANG_SUCCEEDED(otherResult) && deviceOther)
    {
        SLANG_CHECK(bridgeOther != bridgeA); // different key -> different bridge
    }

    // cleanCache drops the cached device (and our reference to its bridge); re-acquiring the same
    // key now creates a fresh device wired to a fresh bridge.
    DeviceCache::cleanCache();
    Slang::ComPtr<rhi::IDevice> deviceAfterClean;
    Slang::RefPtr<CoreToRHIDebugBridge> bridgeAfterClean;
    SlangResult afterCleanResult =
        DeviceCache::acquireDevice(desc, deviceAfterClean.writeRef(), &bridgeAfterClean);
    if (SLANG_SUCCEEDED(afterCleanResult) && deviceAfterClean)
    {
        SLANG_CHECK(bridgeAfterClean != bridgeA); // fresh bridge after cleanCache
    }

    // Don't leave a device in the process-global cache for later tests in this module.
    DeviceCache::cleanCache();
}

SLANG_UNIT_TEST(coreDebugBridgeHandlesConcurrentMessages)
{
    static constexpr int kThreadCount = 4;
    static constexpr int kMessageCount = 1024;

    renderer_test::CoreToRHIDebugBridge bridge;
    renderer_test::CoreDebugCallback callback;
    std::atomic<bool> startWriting(false);
    std::atomic<bool> keepReading(true);

    std::thread readerThread(
        [&]()
        {
            while (keepReading.load(std::memory_order_acquire))
            {
                callback.getString();
            }
        });

    std::thread writerThreads[kThreadCount];
    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
    {
        writerThreads[threadIndex] = std::thread(
            [&]()
            {
                while (!startWriting.load(std::memory_order_acquire))
                {
                    std::this_thread::yield();
                }

                for (int messageIndex = 0; messageIndex < kMessageCount; ++messageIndex)
                {
                    emitError(bridge, "x");
                }
            });
    }

    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &callback);
        startWriting.store(true, std::memory_order_release);
        for (int spinCount = 0; spinCount < 100000 && callback.getString().getLength() == 0;
             ++spinCount)
        {
            std::this_thread::yield();
        }
        SLANG_CHECK(callback.getString().getLength() > 0);
    }

    for (auto& writerThread : writerThreads)
    {
        writerThread.join();
    }

    keepReading.store(false, std::memory_order_release);
    readerThread.join();

    auto capturedLength = callback.getString().getLength();
    SLANG_CHECK(capturedLength > 0);
    SLANG_CHECK(capturedLength <= kThreadCount * kMessageCount * 2);
    SLANG_CHECK((capturedLength % 2) == 0);
}
