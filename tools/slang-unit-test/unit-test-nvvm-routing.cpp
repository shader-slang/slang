// unit-test-nvvm-routing.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-pass-through.h"
#include "slang/slang-session.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;

static slang::CompilerOptionEntry makeCUDAEmissionMethodOption(SlangEmitCUDAMethod method)
{
    slang::CompilerOptionEntry option = {};
    option.name = slang::CompilerOptionName::EmitCUDAMethod;
    option.value.kind = slang::CompilerOptionValueKind::Int;
    option.value.intValue0 = int(method);
    return option;
}

static PassThroughMode resolvePTXDownstreamCompiler(
    slang::IGlobalSession* globalSession,
    bool hasMethod,
    SlangEmitCUDAMethod method)
{
    slang::CompilerOptionEntry methodOption = makeCUDAEmissionMethodOption(method);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    targetDesc.compilerOptionEntryCount = hasMethod ? 1 : 0;
    targetDesc.compilerOptionEntries = hasMethod ? &methodOption : nullptr;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);
    auto linkage = static_cast<Linkage*>(session.get());
    SLANG_CHECK_ABORT(linkage->targets.getCount() == 1);
    return getDownstreamCompilerRequiredForPTXTarget(
        linkage->targets[0]->getOptionSet().getEmitCUDAMethod(),
        globalSession);
}

static ComPtr<slang::IComponentType> createMinimalPTXProgram(
    slang::IGlobalSession* globalSession,
    ComPtr<slang::ISession>& outSession,
    slang::CompilerOptionEntry* targetOption = nullptr)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    targetDesc.compilerOptionEntryCount = targetOption ? 1 : 0;
    targetDesc.compilerOptionEntries = targetOption;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, outSession.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(outSession->loadModuleFromSourceString(
        "nvvmRouting",
        "nvvm-routing.slang",
        "[shader(\"compute\")] [numthreads(1, 1, 1)] void computeMain() {}",
        diagnostics.writeRef()));
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(
        module->findAndCheckEntryPoint(
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[] = {module.get(), entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        outSession->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(program != nullptr);
    return program;
}

static UnownedStringSlice getBlobSlice(slang::IBlob* blob)
{
    SLANG_CHECK_ABORT(blob != nullptr);
    return UnownedStringSlice((const char*)blob->getBufferPointer(), blob->getBufferSize());
}

SLANG_UNIT_TEST(cudaEmissionMethodSelectsDownstreamCompiler)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // A default selection follows the mutable transition and therefore must use the same compiler
    // identity in code generation and the cache hash. Explicit selections bypass this transition.
    globalSession->setDownstreamCompilerForTransition(
        SLANG_CUDA_SOURCE,
        SLANG_PTX,
        SLANG_PASS_THROUGH_DXC);
    SLANG_CHECK(
        resolvePTXDownstreamCompiler(globalSession, false, SLANG_EMIT_CUDA_DEFAULT) ==
        PassThroughMode::Dxc);
    SLANG_CHECK(
        resolvePTXDownstreamCompiler(globalSession, true, SLANG_EMIT_CUDA_VIA_NVRTC) ==
        PassThroughMode::NVRTC);
    SLANG_CHECK(
        resolvePTXDownstreamCompiler(globalSession, true, SLANG_EMIT_CUDA_VIA_NVVM) ==
        PassThroughMode::NVVM);

    // An invalid API-provided value must reach the diagnostic boundary rather than silently
    // selecting either compiler.
    SLANG_CHECK(
        resolvePTXDownstreamCompiler(globalSession, true, SlangEmitCUDAMethod(-1)) ==
        PassThroughMode::None);
}

SLANG_UNIT_TEST(cudaEmissionMethodLinkOptionsAffectRoutingAndHash)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    ComPtr<slang::ISession> nvrtcSession;
    ComPtr<slang::ISession> nvvmSession;
    ComPtr<slang::IComponentType> nvrtcInput = createMinimalPTXProgram(globalSession, nvrtcSession);
    ComPtr<slang::IComponentType> nvvmInput = createMinimalPTXProgram(globalSession, nvvmSession);

    slang::CompilerOptionEntry nvrtcOption =
        makeCUDAEmissionMethodOption(SLANG_EMIT_CUDA_VIA_NVRTC);
    slang::CompilerOptionEntry nvvmOption = makeCUDAEmissionMethodOption(SLANG_EMIT_CUDA_VIA_NVVM);
    ComPtr<slang::IComponentType> nvrtcProgram;
    ComPtr<slang::IComponentType> nvvmProgram;
    SLANG_CHECK_ABORT(
        nvrtcInput->linkWithOptions(nvrtcProgram.writeRef(), 1, &nvrtcOption, nullptr) == SLANG_OK);
    SLANG_CHECK_ABORT(
        nvvmInput->linkWithOptions(nvvmProgram.writeRef(), 1, &nvvmOption, nullptr) == SLANG_OK);

    ComPtr<slang::IBlob> nvrtcHash;
    ComPtr<slang::IBlob> nvvmHash;
    nvrtcProgram->getEntryPointHash(0, 0, nvrtcHash.writeRef());
    nvvmProgram->getEntryPointHash(0, 0, nvvmHash.writeRef());
    SLANG_CHECK_ABORT(nvrtcHash != nullptr && nvvmHash != nullptr);
    SLANG_CHECK_ABORT(nvrtcHash->getBufferSize() == nvvmHash->getBufferSize());
    SLANG_CHECK(
        ::memcmp(
            nvrtcHash->getBufferPointer(),
            nvvmHash->getBufferPointer(),
            nvrtcHash->getBufferSize()) != 0);

    // The link-time option is part of the TargetProgram's effective option set. If dispatch read
    // only TargetRequest, this would silently take the default NVRTC route instead of E52014.
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SlangResult result =
        nvvmProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(code == nullptr);
    SLANG_CHECK(getBlobSlice(diagnostics).indexOf(toSlice("E52014")) != -1);
}

SLANG_UNIT_TEST(invalidCUDAEmissionMethodIsDiagnosed)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::CompilerOptionEntry invalidOption =
        makeCUDAEmissionMethodOption(SlangEmitCUDAMethod(-1));
    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> program =
        createMinimalPTXProgram(globalSession, session, &invalidOption);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(program->link(linkedProgram.writeRef(), nullptr) == SLANG_OK);
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SlangResult result =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(code == nullptr);
    SLANG_CHECK(getBlobSlice(diagnostics).indexOf(toSlice("E52015")) != -1);
}
