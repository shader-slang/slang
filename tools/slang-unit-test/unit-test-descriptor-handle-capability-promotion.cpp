// unit-test-descriptor-handle-capability-promotion.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Laying out a `DescriptorHandle` on a target with no profile or capability promotes the
// `descriptor_handle` capability into that target's capabilities, and the target is shared by every
// program compiled in the session. CUDA already supports descriptor handles, so the promotion must
// leave its caps alone: a later, unrelated closest-hit program must still declare the
// `BuiltInTriangleIntersectionAttributes` struct rather than use the HLSL builtin name.
// See shader-slang/slang#13329.

static const char* _getDescriptorHandleSource()
{
    return R"(
        uniform DescriptorHandle<RWStructuredBuffer<uint>> gCounter;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        void computeMain()
        {
            gCounter[0] = 1;
        }
    )";
}

static const char* _getClosestHitSource()
{
    return R"(
        struct Payload
        {
            float4 color;
        };

        [shader("closesthit")]
        void closestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attribs)
        {
            payload.color = float4(attribs.barycentrics, 0.0, 1.0);
        }
    )";
}

static ComPtr<slang::IComponentType> _linkEntryPoint(
    slang::ISession* session,
    const char* moduleName,
    const char* source,
    const char* entryPointName)
{
    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName,
        (String(moduleName) + ".slang").getBuffer(),
        source,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName(entryPointName, entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);
    return linkedProgram;
}

static void _checkClosestHitCode(slang::ISession* session)
{
    auto program = _linkEntryPoint(session, "closestHit", _getClosestHitSource(), "closestHit");

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnosticBlob;
    SLANG_CHECK_ABORT(
        program->getEntryPointCode(0, 0, code.writeRef(), diagnosticBlob.writeRef()) == SLANG_OK);

    // The CUDA target has no builtin `BuiltInTriangleIntersectionAttributes`, so the emitted
    // code must declare the struct itself rather than refer to the HLSL builtin by name.
    UnownedStringSlice text((const char*)code->getBufferPointer(), code->getBufferSize());
    SLANG_CHECK(text.indexOf(toSlice("struct BuiltInTriangleIntersectionAttributes_0")) != -1);
}

static ComPtr<slang::ISession> _createProfileLessCUDASession(slang::IGlobalSession* globalSession)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_CUDA_SOURCE;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    return session;
}

SLANG_UNIT_TEST(descriptorHandleCapabilityPromotionDoesNotAffectLaterCUDAPrograms)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;

    // Baseline: with no DescriptorHandle in the session, the promotion never runs.
    {
        auto session = _createProfileLessCUDASession(globalSession);
        _checkClosestHitCode(session);
    }

    // Laying out `gCounter` runs the promotion, which writes the session's shared target caps.
    {
        auto session = _createProfileLessCUDASession(globalSession);
        auto descriptorHandleProgram = _linkEntryPoint(
            session,
            "descriptorHandle",
            _getDescriptorHandleSource(),
            "computeMain");
        ComPtr<slang::IBlob> diagnosticBlob;
        SLANG_CHECK_ABORT(descriptorHandleProgram->getLayout(0, diagnosticBlob.writeRef()));
        _checkClosestHitCode(session);
    }
}
