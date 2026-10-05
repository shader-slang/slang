// unit-test-nvrtc-downstream-args-levels.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// `o[1] * o[2] + o[3]` contracts to `fma.rn` unless NVRTC receives `--fmad=false`, so the PTX shows
// whether that argument arrived independently of the `.target` line the architecture argument
// selects.
static const char kFmaKernel[] = R"(
    RWStructuredBuffer<float> o;
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain()
    {
        o[0] = o[1] * o[2] + o[3];
    }
    )";

static slang::CompilerOptionEntry makeNvrtcArg(const char* arg)
{
    slang::CompilerOptionEntry entry = {};
    entry.name = slang::CompilerOptionName::DownstreamArgs;
    entry.value.kind = slang::CompilerOptionValueKind::String;
    entry.value.stringValue0 = "nvrtc";
    entry.value.stringValue1 = arg;
    return entry;
}

// Compile `kFmaKernel` to PTX for the `_cuda_sm_8_0` capability, passing NVRTC arguments at the
// session, target and link levels.
static SlangResult compileToPTX(
    slang::IGlobalSession* globalSession,
    const char* sessionArg,
    const char* targetArg,
    const char* linkArg,
    String& outPTX)
{
    List<slang::CompilerOptionEntry> targetOptions;
    slang::CompilerOptionEntry capability = {};
    capability.name = slang::CompilerOptionName::Capability;
    capability.value.kind = slang::CompilerOptionValueKind::Int;
    capability.value.intValue0 = globalSession->findCapability("_cuda_sm_8_0");
    targetOptions.add(capability);
    if (targetArg)
        targetOptions.add(makeNvrtcArg(targetArg));

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    targetDesc.compilerOptionEntries = targetOptions.getBuffer();
    targetDesc.compilerOptionEntryCount = uint32_t(targetOptions.getCount());

    slang::CompilerOptionEntry sessionOption = makeNvrtcArg(sessionArg ? sessionArg : "");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = sessionArg ? &sessionOption : nullptr;
    sessionDesc.compilerOptionEntryCount = sessionArg ? 1 : 0;

    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", kFmaKernel, diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("computeMain", entryPoint.writeRef()));

    slang::IComponentType* components[] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composite;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        2,
        composite.writeRef(),
        diagnostics.writeRef()));

    slang::CompilerOptionEntry linkOption = makeNvrtcArg(linkArg ? linkArg : "");
    ComPtr<slang::IComponentType> linked;
    SLANG_RETURN_ON_FAIL(composite->linkWithOptions(
        linked.writeRef(),
        linkArg ? 1 : 0,
        linkArg ? &linkOption : nullptr,
        diagnostics.writeRef()));

    ComPtr<slang::IBlob> code;
    SLANG_RETURN_ON_FAIL(linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()));
    outPTX = String(
        UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize()));
    return SLANG_OK;
}

static bool hasTarget(const String& ptx, const char* targetLine)
{
    return ptx.indexOf(UnownedStringSlice(targetLine)) != -1;
}

static bool hasFma(const String& ptx)
{
    return ptx.indexOf(UnownedStringSlice("fma.rn")) != -1;
}

// NVRTC arguments given at different levels (session, target description, `linkWithOptions`) all
// reach NVRTC: adding an unrelated argument at a higher level keeps the architecture selected at
// a lower level, and vice versa. NVRTC compiles to PTX without a GPU, so the test only needs a
// loadable NVRTC; otherwise it reports Ignored.
SLANG_UNIT_TEST(nvrtcDownstreamArgsComposeAcrossLevels)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    const char* arch = "--gpu-architecture=compute_86";
    const char* noFmad = "--fmad=false";

    // Baselines: the capability selects sm_80 and fma contraction is on by default; each argument
    // on its own takes effect.
    String ptx;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, nullptr, nullptr, nullptr, ptx)));
    SLANG_CHECK(hasTarget(ptx, ".target sm_80"));
    SLANG_CHECK(hasFma(ptx));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, arch, nullptr, nullptr, ptx)));
    SLANG_CHECK(hasTarget(ptx, ".target sm_86"));
    SLANG_CHECK(hasFma(ptx));

    // Session architecture + link-time `--fmad=false` (the reported case).
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, arch, nullptr, noFmad, ptx)));
    SLANG_CHECK(hasTarget(ptx, ".target sm_86"));
    SLANG_CHECK(!hasFma(ptx));

    // Session architecture + target-level `--fmad=false`, without any link-time option.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, arch, noFmad, nullptr, ptx)));
    SLANG_CHECK(hasTarget(ptx, ".target sm_86"));
    SLANG_CHECK(!hasFma(ptx));

    // Target-level architecture + link-time `--fmad=false`.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, nullptr, arch, noFmad, ptx)));
    SLANG_CHECK(hasTarget(ptx, ".target sm_86"));
    SLANG_CHECK(!hasFma(ptx));

    // Session `--fmad=false` + link-time architecture.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, noFmad, nullptr, arch, ptx)));
    SLANG_CHECK(hasTarget(ptx, ".target sm_86"));
    SLANG_CHECK(!hasFma(ptx));

    // The same argument at two levels reaches NVRTC once; NVRTC rejects a repeated `--fmad`.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(globalSession, noFmad, nullptr, noFmad, ptx)));
    SLANG_CHECK(!hasFma(ptx));
}
