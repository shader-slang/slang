// unit-test-nvrtc-downstream-args-levels.cpp

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <initializer_list>

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

// NVRTC compiles `FOO * 100 + BAR` only when both macros are defined, and folds it to the constant
// 102 for `FOO=1` and `BAR=2`, so the PTX shows that every `-D` argument arrived.
static const char kMacroKernel[] = R"slang(
    int fooPlusBar()
    {
        __target_switch
        {
        case cuda: __intrinsic_asm "(FOO * 100 + BAR)";
        }
    }
    RWStructuredBuffer<int> o;
    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain()
    {
        o[0] = fooPlusBar();
    }
    )slang";

typedef std::initializer_list<const char*> Args;

static void addNvrtcArgs(List<slang::CompilerOptionEntry>& entries, Args args)
{
    for (auto arg : args)
    {
        slang::CompilerOptionEntry entry = {};
        entry.name = slang::CompilerOptionName::DownstreamArgs;
        entry.value.kind = slang::CompilerOptionValueKind::String;
        entry.value.stringValue0 = "nvrtc";
        entry.value.stringValue1 = arg;
        entries.add(entry);
    }
}

// Compile `source` to PTX for the `_cuda_sm_8_0` capability, passing one NVRTC argument per entry
// at the session, target and link levels, plus the other link-time options in `linkExtra`.
static SlangResult compileToPTX(
    slang::IGlobalSession* globalSession,
    const char* source,
    Args sessionArgs,
    Args targetArgs,
    Args linkArgs,
    List<slang::CompilerOptionEntry> const& linkExtra,
    String& outPTX)
{
    List<slang::CompilerOptionEntry> targetOptions;
    slang::CompilerOptionEntry capability = {};
    capability.name = slang::CompilerOptionName::Capability;
    capability.value.kind = slang::CompilerOptionValueKind::Int;
    capability.value.intValue0 = globalSession->findCapability("_cuda_sm_8_0");
    targetOptions.add(capability);
    addNvrtcArgs(targetOptions, targetArgs);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    targetDesc.compilerOptionEntries = targetOptions.getBuffer();
    targetDesc.compilerOptionEntryCount = uint32_t(targetOptions.getCount());

    List<slang::CompilerOptionEntry> sessionOptions;
    addNvrtcArgs(sessionOptions, sessionArgs);
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = sessionOptions.getBuffer();
    sessionDesc.compilerOptionEntryCount = uint32_t(sessionOptions.getCount());

    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "m.slang", source, diagnostics.writeRef());
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

    List<slang::CompilerOptionEntry> linkOptions;
    addNvrtcArgs(linkOptions, linkArgs);
    linkOptions.addRange(linkExtra);
    ComPtr<slang::IComponentType> linked;
    SLANG_RETURN_ON_FAIL(composite->linkWithOptions(
        linked.writeRef(),
        uint32_t(linkOptions.getCount()),
        linkOptions.getBuffer(),
        diagnostics.writeRef()));

    ComPtr<slang::IBlob> code;
    SLANG_RETURN_ON_FAIL(linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()));
    outPTX =
        String(UnownedStringSlice((const char*)code->getBufferPointer(), code->getBufferSize()));
    return SLANG_OK;
}

static bool contains(const String& ptx, const char* text)
{
    return ptx.indexOf(UnownedStringSlice(text)) != -1;
}

// NVRTC arguments given at different levels (session, target description, `linkWithOptions`) all
// reach NVRTC, session level first: adding an unrelated argument at one level keeps the arguments
// of the others, and arguments given one token per entry arrive exactly as given. NVRTC compiles
// to PTX without a GPU, so the test only needs a loadable NVRTC; otherwise it reports Ignored.
SLANG_UNIT_TEST(nvrtcDownstreamArgsComposeAcrossLevels)
{
    slang::IGlobalSession* globalSession = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        SLANG_IGNORE_TEST;
    }

    const char* arch = "--gpu-architecture=compute_86";
    const char* noFmad = "--fmad=false";
    const List<slang::CompilerOptionEntry> noLinkExtra;
    String ptx;

    // Baselines: the capability selects sm_80 and fma contraction is on by default.
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(compileToPTX(globalSession, kFmaKernel, {}, {}, {}, noLinkExtra, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_80"));
    SLANG_CHECK(contains(ptx, "fma.rn"));

    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(compileToPTX(globalSession, kFmaKernel, {arch}, {}, {}, noLinkExtra, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));
    SLANG_CHECK(contains(ptx, "fma.rn"));

    // The reported cases: a link-time `--fmad=false`, and a link-time option that is not a
    // downstream argument, both keep the session architecture.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileToPTX(globalSession, kFmaKernel, {arch}, {}, {noFmad}, noLinkExtra, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));
    SLANG_CHECK(!contains(ptx, "fma.rn"));

    List<slang::CompilerOptionEntry> optimization;
    slang::CompilerOptionEntry optimizationEntry = {};
    optimizationEntry.name = slang::CompilerOptionName::Optimization;
    optimizationEntry.value.kind = slang::CompilerOptionValueKind::Int;
    optimizationEntry.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    optimization.add(optimizationEntry);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileToPTX(globalSession, kFmaKernel, {arch}, {}, {}, optimization, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileToPTX(globalSession, kFmaKernel, {arch}, {noFmad}, {}, noLinkExtra, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));
    SLANG_CHECK(!contains(ptx, "fma.rn"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileToPTX(globalSession, kFmaKernel, {}, {arch}, {noFmad}, noLinkExtra, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));
    SLANG_CHECK(!contains(ptx, "fma.rn"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileToPTX(globalSession, kFmaKernel, {noFmad}, {}, {arch}, noLinkExtra, ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));
    SLANG_CHECK(!contains(ptx, "fma.rn"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(
        globalSession,
        kMacroKernel,
        {"-DFOO=1"},
        {arch, "-DBAR=2"},
        {noFmad},
        noLinkExtra,
        ptx)));
    SLANG_CHECK(contains(ptx, ".target sm_86"));
    SLANG_CHECK(contains(ptx, " 102;"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(
        globalSession,
        kMacroKernel,
        {"-D", "FOO=1", "-D", "BAR=2"},
        {},
        {},
        noLinkExtra,
        ptx)));
    SLANG_CHECK(contains(ptx, " 102;"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(
        globalSession,
        kMacroKernel,
        {},
        {},
        {"-D", "FOO=1", "-D", "BAR=2"},
        noLinkExtra,
        ptx)));
    SLANG_CHECK(contains(ptx, " 102;"));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileToPTX(
        globalSession,
        kMacroKernel,
        {"-D", "FOO=1"},
        {},
        {"-D", "BAR=2"},
        noLinkExtra,
        ptx)));
    SLANG_CHECK(contains(ptx, " 102;"));
}
