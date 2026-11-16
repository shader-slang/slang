#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-nvdisasm-compiler.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "compiler-core/slang-slice-allocator.h"
#include "core/slang-blob.h"
#include "core/slang-shared-library.h"
#include "core/slang-type-text-util.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// Compile a public API program so resource layout and CUDA lowering are exercised along with
// the downstream adapter. The same source is used for binary and assembly output.
SlangResult compileCubinProgram(
    slang::IGlobalSession* global,
    SlangCompileTarget target,
    ComPtr<slang::IBlob>& code,
    ComPtr<slang::IBlob>& diagnostics)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = target;
    slang::CompilerOptionEntry capability = {};
    capability.name = slang::CompilerOptionName::Capability;
    capability.value.kind = slang::CompilerOptionValueKind::Int;
    capability.value.intValue0 = global->findCapability("cuda_sm_8_0");
    targetDesc.compilerOptionEntries = &capability;
    targetDesc.compilerOptionEntryCount = 1;
    slang::SessionDesc desc = {};
    desc.targets = &targetDesc;
    desc.targetCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(global->createSession(desc, session.writeRef()));
    auto module = session->loadModuleFromSourceString(
        "cubin",
        "cubin.slang",
        R"(
        RWStructuredBuffer<uint> outputBuffer;
        [shader("compute")]
        [numthreads(32, 1, 1)]
        void computeMain(uint3 tid : SV_DispatchThreadID)
        {
            outputBuffer[tid.x] = tid.x + 1;
        }
    )",
        diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;
    ComPtr<slang::IEntryPoint> entry;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("computeMain", entry.writeRef()));
    slang::IComponentType* components[] = {module, entry};
    ComPtr<slang::IComponentType> program;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        2,
        program.writeRef(),
        diagnostics.writeRef()));
    ComPtr<slang::IComponentType> linked;
    SLANG_RETURN_ON_FAIL(program->link(linked.writeRef(), diagnostics.writeRef()));
    return linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
}

enum class CubinAPIState
{
    Missing,
    Empty,
    RetrievalFailure,
};

// Wrap a real library at the existing shared-library seam. This keeps compilation real while
// exercising old toolkits and retrieval failures without requiring those installations on CI.
class CubinLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    CubinLibrary(ISlangSharedLibrary* library, CubinAPIState state)
        : m_library(library), m_state(state)
    {
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }
    SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(const char* name) SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);
        if (symbol == "nvrtcGetCUBINSize")
        {
            if (m_state == CubinAPIState::Missing)
                return nullptr;
            if (m_state == CubinAPIState::Empty)
                return (void*)emptyCubin;
        }
        if (symbol == "nvrtcGetCUBIN")
        {
            if (m_state == CubinAPIState::Missing)
                return nullptr;
            if (m_state == CubinAPIState::RetrievalFailure)
                return (void*)failCubin;
        }
        return m_library->findSymbolAddressByName(name);
    }

private:
    static int emptyCubin(void*, size_t* size)
    {
        *size = 0;
        return 0;
    }
    static int failCubin(void*, char*) { return 3; } // NVRTC_ERROR_INVALID_INPUT
    void* getInterface(const Guid& guid)
    {
        return guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                       guid == ISlangSharedLibrary::getTypeGuid()
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }
    ComPtr<ISlangSharedLibrary> m_library;
    CubinAPIState m_state;
};

class CubinLibraryLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    CubinLibraryLoader(ISlangSharedLibrary* library)
        : m_library(library)
    {
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(const char*, ISlangSharedLibrary** out)
        SLANG_OVERRIDE
    {
        m_library->addRef();
        *out = m_library;
        return SLANG_OK;
    }

private:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return guid == ISlangUnknown::getTypeGuid() ||
                       guid == ISlangSharedLibraryLoader::getTypeGuid()
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
    ComPtr<ISlangSharedLibrary> m_library;
};

SlangResult compileCudaSource(
    IDownstreamCompiler* compiler,
    SlangCompileTarget target,
    ComPtr<IArtifact>& artifact)
{
    auto source = ArtifactUtil::createArtifactForCompileTarget(SLANG_CUDA_SOURCE);
    source->addRepresentationUnknown(
        StringBlob::create(String("extern \"C\" __global__ void computeMain() {}")));
    IArtifact* sources[] = {source};
    String architecture("-arch=sm_80");
    auto argument = SliceUtil::asTerminatedCharSlice(architecture);
    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
    options.targetType = target;
    options.sourceArtifacts = makeSlice(sources, 1);
    options.compilerSpecificArguments = makeSlice(&argument, 1);
    return compiler->compile(options, artifact.writeRef());
}
} // namespace

SLANG_UNIT_TEST(cubinArtifactMappings)
{
    auto binary = ArtifactDescUtil::makeDescForCompileTarget(SLANG_CUDA_OBJECT_CODE);
    auto assembly = ArtifactDescUtil::makeDescForCompileTarget(SLANG_CUDA_OBJECT_CODE_ASM);
    SLANG_CHECK(binary.kind == ArtifactKind::ObjectCode);
    SLANG_CHECK(binary.payload == ArtifactPayload::CuBin);
    SLANG_CHECK(ArtifactDescUtil::isGpuUsable(binary));
    SLANG_CHECK(!ArtifactDescUtil::isText(binary));
    SLANG_CHECK(ArtifactDescUtil::isText(assembly));
    SLANG_CHECK(ArtifactDescUtil::isDisassembly(binary, assembly));
    SLANG_CHECK(ArtifactDescUtil::getCompileTargetFromDesc(binary) == SLANG_CUDA_OBJECT_CODE);
    SLANG_CHECK(ArtifactDescUtil::getCompileTargetFromDesc(assembly) == SLANG_CUDA_OBJECT_CODE_ASM);
    SLANG_CHECK(
        TypeTextUtil::findCompileTargetFromName(UnownedStringSlice("cubin")) ==
        SLANG_CUDA_OBJECT_CODE);
    SLANG_CHECK(
        TypeTextUtil::findCompileTargetFromName(UnownedStringSlice("cuobj")) ==
        SLANG_CUDA_OBJECT_CODE);
    SLANG_CHECK(ArtifactDescUtil::getDescFromPath(UnownedStringSlice("kernel.cubin")) == binary);
    SLANG_CHECK(
        ArtifactDescUtil::getDescFromPath(UnownedStringSlice("kernel.cubin.asm")) == assembly);
    StringBuilder binaryExtension, assemblyExtension;
    SLANG_CHECK(SLANG_SUCCEEDED(ArtifactDescUtil::appendDefaultExtension(binary, binaryExtension)));
    SLANG_CHECK(
        SLANG_SUCCEEDED(ArtifactDescUtil::appendDefaultExtension(assembly, assemblyExtension)));
    SLANG_CHECK(binaryExtension == "cubin");
    SLANG_CHECK(assemblyExtension == "cubin.asm");
}

SLANG_UNIT_TEST(cubinPublicAPI)
{
    auto global = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE)))
    {
        SLANG_IGNORE_TEST;
    }
    ComPtr<slang::IBlob> code, diagnostics;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(compileCubinProgram(global, SLANG_CUDA_OBJECT_CODE, code, diagnostics)));
    SLANG_CHECK_ABORT(code && code->getBufferSize() > 4);
    SLANG_CHECK(
        memcmp(
            code->getBufferPointer(),
            "\x7f"
            "ELF",
            4) == 0);
}

SLANG_UNIT_TEST(cubinDisassemblyAPI)
{
    auto global = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE_ASM)))
    {
        SLANG_IGNORE_TEST;
    }
    ComPtr<slang::IBlob> code, diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        compileCubinProgram(global, SLANG_CUDA_OBJECT_CODE_ASM, code, diagnostics)));
    UnownedStringSlice text((const char*)code->getBufferPointer(), code->getBufferSize());
    SLANG_CHECK(text.indexOf(UnownedStringSlice("computeMain")) >= 0);
    SLANG_CHECK(text.indexOf(UnownedStringSlice(".headerflags")) >= 0);
}

SLANG_UNIT_TEST(cubinRetrievalDiagnostics)
{
    if (SLANG_FAILED(
            unitTestContext->slangGlobalSession->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE)))
    {
        SLANG_IGNORE_TEST;
    }
    ComPtr<ISlangBlob> path;
    if (SLANG_FAILED(unitTestContext->slangGlobalSession->getDownstreamCompilerPath(
            SLANG_PASS_THROUGH_NVRTC,
            path.writeRef())))
    {
        SLANG_IGNORE_TEST;
    }
    ComPtr<ISlangSharedLibrary> realLibrary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        DefaultSharedLibraryLoader::getSingleton()->loadPlatformSharedLibrary(
            (const char*)path->getBufferPointer(),
            realLibrary.writeRef())));
    for (auto state :
         {CubinAPIState::Missing, CubinAPIState::Empty, CubinAPIState::RetrievalFailure})
    {
        ComPtr<ISlangSharedLibrary> library(new CubinLibrary(realLibrary, state));
        ComPtr<ISlangSharedLibraryLoader> loader(new CubinLibraryLoader(library));
        RefPtr<DownstreamCompilerSet> set(new DownstreamCompilerSet());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(NVRTCDownstreamCompilerUtil::locateCompilers(String(), loader, set)));
        List<IDownstreamCompiler*> compilers;
        set->getCompilers(compilers);
        SLANG_CHECK_ABORT(compilers.getCount() == 1);
        ComPtr<IArtifact> artifact;
        compileCudaSource(compilers[0], SLANG_CUDA_OBJECT_CODE, artifact);
        SLANG_CHECK_ABORT(artifact);
        auto diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(artifact);
        SLANG_CHECK_ABORT(diagnostics);
        SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
        SLANG_CHECK(diagnostics->getRaw().count > 0);
        SLANG_CHECK(!compilers[0]->canConvert(
            artifact->getDesc(),
            ArtifactDescUtil::makeDescForCompileTarget(SLANG_CUDA_OBJECT_CODE_ASM)));

        // Missing CUBIN APIs must leave the real PTX compilation and retrieval path usable.
        artifact.setNull();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compilers[0], SLANG_PTX, artifact)));
        diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(artifact);
        SLANG_CHECK_ABORT(diagnostics && SLANG_SUCCEEDED(diagnostics->getResult()));
        ComPtr<ISlangBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(artifact->loadBlob(ArtifactKeep::No, code.writeRef())));
        SLANG_CHECK(code->getBufferSize() > 0);
    }
}

SLANG_UNIT_TEST(cubinMissingDisassembler)
{
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, global.writeRef())));
    global->setDownstreamCompilerPath(
        SLANG_PASS_THROUGH_NVDISASM,
        "missing-cubin-test-tool/nvdisasm");
    SLANG_CHECK(SLANG_FAILED(global->checkPassThroughSupport(SLANG_PASS_THROUGH_NVDISASM)));
}

SLANG_UNIT_TEST(cubinMissingDisassemblerOutput)
{
    if (SLANG_FAILED(
            unitTestContext->slangGlobalSession->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE)))
    {
        SLANG_IGNORE_TEST;
    }
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(slang_createGlobalSession(SLANG_API_VERSION, global.writeRef())));
    global->setDownstreamCompilerPath(
        SLANG_PASS_THROUGH_NVDISASM,
        "missing-cubin-test-tool/nvdisasm");
    ComPtr<slang::IBlob> code, diagnostics;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(compileCubinProgram(global, SLANG_CUDA_OBJECT_CODE, code, diagnostics)));
    SLANG_CHECK_ABORT(
        SLANG_FAILED(compileCubinProgram(global, SLANG_CUDA_OBJECT_CODE_ASM, code, diagnostics)));
    SLANG_CHECK_ABORT(diagnostics && diagnostics->getBufferSize());
    UnownedStringSlice text(
        (const char*)diagnostics->getBufferPointer(),
        diagnostics->getBufferSize());
    SLANG_CHECK(text.indexOf(UnownedStringSlice("nvdisasm")) >= 0);
}

SLANG_UNIT_TEST(cubinDisassemblerRejectsInvalidInput)
{
    RefPtr<DownstreamCompilerSet> set(new DownstreamCompilerSet());
    if (SLANG_FAILED(NvdisasmDownstreamCompilerUtil::locateCompilers(String(), nullptr, set)))
    {
        SLANG_IGNORE_TEST;
    }
    List<IDownstreamCompiler*> compilers;
    set->getCompilers(compilers);
    SLANG_CHECK_ABORT(compilers.getCount() == 1);
    auto source = ArtifactUtil::createArtifactForCompileTarget(SLANG_CUDA_OBJECT_CODE);
    source->addRepresentationUnknown(StringBlob::create(String("not a CUDA ELF binary")));
    auto assemblyDesc = ArtifactDescUtil::makeDescForCompileTarget(SLANG_CUDA_OBJECT_CODE_ASM);
    SLANG_CHECK(compilers[0]->canConvert(source->getDesc(), assemblyDesc));
    SLANG_CHECK(!compilers[0]->canConvert(assemblyDesc, source->getDesc()));
    SLANG_CHECK(!compilers[0]->canConvert(
        ArtifactDescUtil::makeDescForCompileTarget(SLANG_PTX),
        assemblyDesc));
    ComPtr<IArtifact> assembly;
    SLANG_CHECK(SLANG_FAILED(compilers[0]->convert(source, assemblyDesc, assembly.writeRef())));
    SLANG_CHECK_ABORT(assembly);
    auto diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(assembly);
    SLANG_CHECK_ABORT(diagnostics && SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(asStringSlice(diagnostics->getRaw()).indexOf(UnownedStringSlice("nvdisasm")) >= 0);
}
