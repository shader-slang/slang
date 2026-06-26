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

renderer_test::CoreToRHIDebugBridge& getStaticBridgeAfterStackCallbackScope()
{
    static renderer_test::CoreToRHIDebugBridge bridge;

    renderer_test::CoreDebugCallback callback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &callback);
        emitError(bridge, "static scope");
    }
    SLANG_CHECK(callback.getString() == "static scope\n");

    return bridge;
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

SLANG_UNIT_TEST(scopedCoreDebugCallbackClearsStaticBridgeAfterStackCallback)
{
    renderer_test::CoreToRHIDebugBridge& bridge = getStaticBridgeAfterStackCallbackScope();

    emitError(bridge, "after stack callback");

    renderer_test::CoreDebugCallback nextCallback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &nextCallback);
        emitError(bridge, "next invocation");
    }
    SLANG_CHECK(nextCallback.getString() == "next invocation\n");
}

SLANG_UNIT_TEST(coreDebugBridgeHandlesConcurrentMessages)
{
    static constexpr int kThreadCount = 4;
    static constexpr int kMessageCount = 64;

    renderer_test::CoreToRHIDebugBridge bridge;
    renderer_test::CoreDebugCallback callback;
    {
        renderer_test::ScopedCoreDebugCallback scopedDebugCallback(bridge, &callback);

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
                    for (int messageIndex = 0; messageIndex < kMessageCount; ++messageIndex)
                    {
                        emitError(bridge, "x");
                    }
                });
        }

        for (auto& writerThread : writerThreads)
        {
            writerThread.join();
        }

        keepReading.store(false, std::memory_order_release);
        readerThread.join();
    }

    SLANG_CHECK(callback.getString().getLength() == kThreadCount * kMessageCount * 2);
}
