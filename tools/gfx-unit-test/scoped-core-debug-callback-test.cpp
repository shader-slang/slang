#include "../render-test/slang-support.h"

#include "unit-test/slang-unit-test.h"

namespace
{
void emitError(renderer_test::CoreToRHIDebugBridge& bridge, const char* message)
{
    bridge.handleMessage(rhi::DebugMessageType::Error, rhi::DebugMessageSource::Layer, message);
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
    SLANG_CHECK(true);
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
