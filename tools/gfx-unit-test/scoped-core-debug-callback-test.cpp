#include "../render-test/slang-support.h"
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
