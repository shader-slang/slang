#include "tools/unit-test/slang-unit-test.h"

#include "slang-gfx.h"
#include "gfx-test-util.h"
#include "tools/gfx-util/shader-cursor.h"
#include "source/core/slang-basic.h"

using namespace gfx;

namespace gfx_test
{
    void sharedHandleTestImpl(IDevice* srcDevice, IDevice* dstDevice, UnitTestContext* context)
    {
        const int numberCount = 4;
        float initialData[] = { 0.0f, 1.0f, 2.0f, 3.0f };
        IBufferResource::Desc bufferDesc = {};
        bufferDesc.sizeInBytes = numberCount * sizeof(float);
        bufferDesc.format = gfx::Format::Unknown;
        bufferDesc.elementSize = sizeof(float);
        bufferDesc.allowedStates = ResourceStateSet(
            ResourceState::ShaderResource,
            ResourceState::UnorderedAccess,
            ResourceState::CopyDestination,
            ResourceState::CopySource);
        bufferDesc.defaultState = ResourceState::UnorderedAccess;
        bufferDesc.cpuAccessFlags = AccessFlag::Write | AccessFlag::Read;
        bufferDesc.isShared = true;

        ComPtr<IBufferResource> srcBuffer;
        GFX_CHECK_CALL_ABORT(srcDevice->createBufferResource(
            bufferDesc,
            (void*)initialData,
            srcBuffer.writeRef()));

        InteropHandle sharedHandle;
        GFX_CHECK_CALL_ABORT(srcBuffer->getSharedHandle(&sharedHandle));
        ComPtr<IBufferResource> dstBuffer;
        GFX_CHECK_CALL_ABORT(dstDevice->createBufferFromSharedHandle(sharedHandle, bufferDesc, dstBuffer.writeRef()));

        InteropHandle testHandle;
        GFX_CHECK_CALL_ABORT(dstBuffer->getNativeResourceHandle(&testHandle));
        IBufferResource::Desc* testDesc = dstBuffer->getDesc();
        SLANG_CHECK(testDesc->elementSize == sizeof(float));
        SLANG_CHECK(testDesc->sizeInBytes == numberCount * sizeof(float));
        compareComputeResult(dstDevice, dstBuffer, Slang::makeArray<float>(0.0f, 1.0f, 2.0f, 3.0f));
    }

    void sharedHandleTestAPI(UnitTestContext* context, Slang::RenderApiFlag::Enum srcApi, Slang::RenderApiFlag::Enum dstApi)
    {
        auto srcDevice = createTestingDevice(context, srcApi);
        auto dstDevice = createTestingDevice(context, dstApi);
        if (!srcDevice || !dstDevice)
        {
            SLANG_IGNORE_TEST;
        }

        sharedHandleTestImpl(srcDevice, dstDevice, context);
    }

    // Temporarily disabled due to inconsistent test results on TC
    SLANG_UNIT_TEST(sharedHandleD3D12ToCUDA)
    {
        sharedHandleTestAPI(unitTestContext, Slang::RenderApiFlag::D3D12, Slang::RenderApiFlag::CUDA);
    }

    SLANG_UNIT_TEST(sharedHandleVulkanToCUDA)
    {
        sharedHandleTestAPI(unitTestContext, Slang::RenderApiFlag::Vulkan, Slang::RenderApiFlag::CUDA);
    }
}
