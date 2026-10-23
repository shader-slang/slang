// unit-test-argument-buffer-tier-2-reflection.cpp

#include "core/slang-io.h"
#include "core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

// Test metal argument buffer tier2 layout rules.

SLANG_UNIT_TEST(metalArgumentBufferTier2Reflection)
{
    const char* userSourceBody = R"(
        struct A
        {
          float3 one;
          float3 two;
          float three;
        }

        struct Args{
          ParameterBlock<A> a;
        }
        ParameterBlock<Args> argument_buffer;
        RWStructuredBuffer<float> outputBuffer;

        [numthreads(1,1,1)]
        void computeMain()
        {
            outputBuffer[0] = argument_buffer.a.two.x;
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "m",
        "m.slang",
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    auto layout = module->getLayout();

    auto type = layout->findTypeByName("A");
    auto typeLayout = layout->getTypeLayout(type, slang::LayoutRules::MetalArgumentBufferTier2);
    SLANG_CHECK(typeLayout->getFieldByIndex(0)->getOffset() == 0);
    SLANG_CHECK(typeLayout->getFieldByIndex(0)->getTypeLayout()->getSize() == 16);
    SLANG_CHECK(typeLayout->getFieldByIndex(1)->getOffset() == 16);
    SLANG_CHECK(typeLayout->getFieldByIndex(1)->getTypeLayout()->getSize() == 16);
    SLANG_CHECK(typeLayout->getFieldByIndex(2)->getOffset() == 32);
    SLANG_CHECK(typeLayout->getFieldByIndex(2)->getTypeLayout()->getSize() == 4);
}

SLANG_UNIT_TEST(metalTier2SubpassInputLayout)
{
    // A SubpassInput carries three units under the tier 2 request, the texture bytes, the
    // texture slot, and the input attachment index. Merging the texture and subpass layouts
    // once overflowed an object layout that holds only two.
    const char* src = R"(
        struct Foo { [[vk::input_attachment_index(0)]] SubpassInput<float4> sp; float s; }
    )";
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_METAL;
    targetDesc.profile = globalSession->findProfile("metal");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    ComPtr<slang::IBlob> diag;
    auto module = session->loadModuleFromSourceString("m", "m.slang", src, diag.writeRef());
    SLANG_CHECK(module != nullptr);
    auto layout = module->getLayout();
    auto foo = layout->findTypeByName("Foo");
    auto tl = layout->getTypeLayout(foo, slang::LayoutRules::MetalArgumentBufferTier2);
    auto sp = tl->getFieldByIndex(0)->getTypeLayout();
    SLANG_CHECK(sp->getCategoryCount() == 3);
    SLANG_CHECK(sp->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) == 8);
    SLANG_CHECK(sp->getSize(SLANG_PARAMETER_CATEGORY_METAL_ARGUMENT_BUFFER_ELEMENT) == 2);
    SLANG_CHECK(sp->getSize(SLANG_PARAMETER_CATEGORY_SUBPASS) == 1);
    // 'float s' follows the SubpassInput, landing at byte offset 8 past the texture bytes and slot
    // index 2 past the two argument buffer element slots.
    auto sField = tl->getFieldByIndex(1);
    SLANG_CHECK(sField->getOffset() == 8);
    SLANG_CHECK(sField->getOffset(SLANG_PARAMETER_CATEGORY_METAL_ARGUMENT_BUFFER_ELEMENT) == 2);
}

SLANG_UNIT_TEST(metalTier2StructuredBufferDescriptorRange)
{
    // A structured buffer inside a tier 2 argument buffer is a device pointer, so its descriptor
    // range must report the byte offset where the pointer sits, not the tier 1 slot index. The
    // constant buffer occupies the first eight bytes, so the structured buffer sits at byte eight.
    const char* src = R"(
        struct S { ConstantBuffer<int> cb; RWStructuredBuffer<float> buf; }
    )";
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_METAL;
    targetDesc.profile = globalSession->findProfile("metal");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    ComPtr<slang::IBlob> diag;
    auto module = session->loadModuleFromSourceString("m", "m.slang", src, diag.writeRef());
    SLANG_CHECK(module != nullptr);
    auto layout = module->getLayout();
    auto s = layout->findTypeByName("S");
    auto tl = layout->getTypeLayout(s, slang::LayoutRules::MetalArgumentBufferTier2);
    bool checkedBuffer = false;
    bool checkedConstantBuffer = false;
    for (int r = 0; r < (int)tl->getBindingRangeCount(); ++r)
    {
        auto type = tl->getBindingRangeType(r);
        if (type != slang::BindingType::MutableRawBuffer &&
            type != slang::BindingType::ConstantBuffer)
            continue;
        auto setIndex = tl->getBindingRangeDescriptorSetIndex(r);
        auto rangeIndex = tl->getBindingRangeFirstDescriptorRangeIndex(r);
        auto offset = tl->getDescriptorSetDescriptorRangeIndexOffset(setIndex, rangeIndex);
        if (type == slang::BindingType::MutableRawBuffer)
        {
            SLANG_CHECK(offset == 8);
            checkedBuffer = true;
        }
        else
        {
            SLANG_CHECK(offset == 0);
            checkedConstantBuffer = true;
        }
    }
    SLANG_CHECK(checkedBuffer);
    SLANG_CHECK(checkedConstantBuffer);
    // Each field consumes one argument buffer element slot in order.
    auto cbField = tl->getFieldByIndex(0);
    auto bufField = tl->getFieldByIndex(1);
    SLANG_CHECK(cbField->getOffset(SLANG_PARAMETER_CATEGORY_METAL_ARGUMENT_BUFFER_ELEMENT) == 0);
    SLANG_CHECK(bufField->getOffset(SLANG_PARAMETER_CATEGORY_METAL_ARGUMENT_BUFFER_ELEMENT) == 1);
}
