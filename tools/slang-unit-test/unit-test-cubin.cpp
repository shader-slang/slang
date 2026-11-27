#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-nvdisasm-compiler.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "compiler-core/slang-slice-allocator.h"
#include "core/slang-blob.h"
#include "core/slang-crypto.h"
#include "core/slang-hex-dump-util.h"
#include "core/slang-shared-library.h"
#include "core/slang-string-util.h"
#include "core/slang-type-text-util.h"
#include "core/slang-writer.h"
#include "slang-com-ptr.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// Create a session targeting SM80 for CUBIN or its disassembly.
ComPtr<slang::ISession> createCubinSession(slang::IGlobalSession* global, SlangCompileTarget target)
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
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(global->createSession(desc, session.writeRef())));
    return session;
}

// Load the same kernel used by the CLI tests through the component API.
ComPtr<slang::IEntryPoint> loadComputeEntryPoint(slang::ISession* session)
{
    auto module = session->loadModule("tests/cuda/cubin.slang");
    SLANG_CHECK_ABORT(module);
    ComPtr<slang::IEntryPoint> entry;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", entry.writeRef())));
    return entry;
}

// Return the hex digest so the test can compare the complete binary hashes.
String getEntryPointHash(slang::IEntryPoint* entry)
{
    ComPtr<slang::IBlob> hash;
    entry->getEntryPointHash(0, 0, hash.writeRef());
    SLANG_CHECK_ABORT(hash && hash->getBufferSize());
    return DigestUtil::digestToString(hash->getBufferPointer(), hash->getBufferSize());
}

enum class CubinAPIState
{
    Missing,
    Empty,
    RetrievalFailure,
};

// Wrap a real library at the existing shared-library seam. This keeps compilation real while
// exercising old toolkits and retrieval failures without requiring those installations on CI.
class CubinLibrary : public DefaultSharedLibrary
{
public:
    CubinLibrary(SharedLibrary::Handle handle, CubinAPIState state)
        : DefaultSharedLibrary(handle), m_state(state)
    {
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
        return DefaultSharedLibrary::findSymbolAddressByName(name);
    }

private:
    static int emptyCubin(void*, size_t* size)
    {
        *size = 0;
        return 0;
    }
    static int failCubin(void*, char*) { return 3; } // NVRTC_ERROR_INVALID_INPUT
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

SLANG_UNIT_TEST(cubinArtifactMappings, NoRenderApi)
{
    SLANG_CHECK(
        unitTestContext->slangGlobalSession->getDownstreamCompilerForTransition(
            SLANG_CUDA_OBJECT_CODE,
            SLANG_CUDA_OBJECT_CODE_ASM) == SLANG_PASS_THROUGH_NVDISASM);
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
        TypeTextUtil::findCompileTargetFromName(toSlice("cuobj")) == SLANG_CUDA_OBJECT_CODE);
    SLANG_CHECK(ArtifactDescUtil::getDescFromPath(toSlice("kernel.cubin")) == binary);
    for (const char* path : {"kernel.obj", "kernel.o"})
    {
        const auto ext = TypeTextUtil::getOutputPathExtension(UnownedStringSlice(path));
        SLANG_CHECK(TypeTextUtil::findCompileTargetFromExtension(ext) == SLANG_OBJECT_CODE);
        const auto desc = ArtifactDescUtil::getDescFromPath(UnownedStringSlice(path));
        SLANG_CHECK(ArtifactDescUtil::getCompileTargetFromDesc(desc) == SLANG_OBJECT_CODE);
    }
    SLANG_CHECK(
        ArtifactDescUtil::getDescFromPath(toSlice("kernel.cubin/debug.asm")) ==
        ArtifactDescUtil::getDescFromExtension(toSlice("asm")));
    StringBuilder binaryExtension;
    SLANG_CHECK(SLANG_SUCCEEDED(ArtifactDescUtil::appendDefaultExtension(binary, binaryExtension)));
    SLANG_CHECK(binaryExtension == "cubin");

    for (const char* path : {"dir.with.dots/kernel.debug.cubin.asm", "kernel.debug.cubin-asm"})
    {
        const UnownedStringSlice assemblyPath(path);
        SLANG_CHECK(ArtifactDescUtil::getDescFromPath(assemblyPath) == assembly);
        const String base = ArtifactDescUtil::getBaseNameFromPath(assembly, assemblyPath);
        SLANG_CHECK(base == "kernel.debug");
        StringBuilder name;
        SLANG_CHECK(SLANG_SUCCEEDED(
            ArtifactDescUtil::calcNameForDesc(assembly, base.getUnownedSlice(), name)));
        SLANG_CHECK(name == "kernel.debug.cubin.asm");
    }
}

SLANG_UNIT_TEST(cubinAssemblyHashIncludesDownstreamInputs, NoRenderApi)
{
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(global.writeRef())));
    auto session = createCubinSession(global, SLANG_CUDA_OBJECT_CODE_ASM);
    auto entry = loadComputeEntryPoint(session);

    // The extra kernel becomes part of the CUBIN that nvdisasm reads.
    auto initial = getEntryPointHash(entry);
    global->setLanguagePrelude(
        SLANG_SOURCE_LANGUAGE_CUDA,
        "extern \"C\" __global__ void extra() {}");
    auto withExtraKernel = getEntryPointHash(entry);
    SLANG_CHECK(initial != withExtraKernel);

    if (SLANG_SUCCEEDED(global->checkPassThroughSupport(SLANG_PASS_THROUGH_NVDISASM)))
    {
        // Removing nvdisasm drops its version from the hash while keeping the CUDA inputs fixed.
        global->setDownstreamCompilerPath(
            SLANG_PASS_THROUGH_NVDISASM,
            "missing-cubin-test-tool/nvdisasm");
        SLANG_CHECK(getEntryPointHash(entry) != withExtraKernel);
    }
}

SLANG_UNIT_TEST(cubinSelectedEntryPointStage, CUDA)
{
    auto global = unitTestContext->slangGlobalSession;
    if (SLANG_FAILED(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE)))
    {
        SLANG_IGNORE_TEST;
    }
    auto session = createCubinSession(global, SLANG_CUDA_OBJECT_CODE);
    auto module = session->loadModule("tests/cuda/cubin-raytracing.slang");
    SLANG_CHECK_ABORT(module);
    ComPtr<slang::IEntryPoint> compute, ray;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(module->findEntryPointByName("computeMain", compute.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("rayMain", ray.writeRef())));

    // Request index 1 in both orders so using the stage at index 0 fails in both directions.
    for (auto selected : {compute.get(), ray.get()})
    {
        auto first = selected == compute.get() ? ray.get() : compute.get();
        slang::IComponentType* components[] = {module, first, selected};
        ComPtr<slang::IComponentType> program, linked;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef())));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(program->link(linked.writeRef())));

        ComPtr<slang::IBlob> code, diagnostics;
        auto result = linked->getEntryPointCode(1, 0, code.writeRef(), diagnostics.writeRef());
        if (selected == compute.get())
        {
            SLANG_CHECK(SLANG_SUCCEEDED(result));
            SLANG_CHECK(
                code && code->getBufferSize() >= 4 &&
                memcmp(code->getBufferPointer(), "\177ELF", 4) == 0);
        }
        else
        {
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(
                StringUtil::getSlice(diagnostics).indexOf(toSlice("OptiX ray-tracing pipeline")) >=
                0);
        }
    }
}

SLANG_UNIT_TEST(cubinRetrievalDiagnostics, CUDA)
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
    const struct
    {
        CubinAPIState state;
        UnownedStringSlice expectedError;
    } cases[] = {
        {CubinAPIState::Missing, toSlice("nvrtcGetCUBIN support")},
        {CubinAPIState::Empty, toSlice("NVRTC produced no code")},
        {CubinAPIState::RetrievalFailure, toSlice("Failed to retrieve CUBIN")},
    };
    for (auto [state, expectedError] : cases)
    {
        SharedLibrary::Handle handle;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            SharedLibrary::loadWithPlatformPath((const char*)path->getBufferPointer(), handle)));
        ComPtr<ISlangSharedLibrary> library(new CubinLibrary(handle, state));
        ComPtr<ISlangSharedLibraryLoader> loader(new CubinLibraryLoader(library));
        ComPtr<slang::IGlobalSession> global;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(global.writeRef())));
        global->setSharedLibraryLoader(loader);
        SLANG_CHECK(global->checkCompileTargetSupport(SLANG_PTX) == SLANG_OK);
        const auto expected = state == CubinAPIState::Missing ? SLANG_E_NOT_FOUND : SLANG_OK;
        SLANG_CHECK(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE) == expected);
        if (state == CubinAPIState::Missing)
            SLANG_CHECK(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE_ASM) == expected);

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
        SLANG_CHECK(asStringSlice(diagnostics->getRaw()).indexOf(expectedError) >= 0);
        SLANG_CHECK(!compilers[0]->canConvert(
            artifact->getDesc(),
            ArtifactDescUtil::makeDescForCompileTarget(SLANG_CUDA_OBJECT_CODE_ASM)));

        // Missing CUBIN APIs must leave the real PTX compilation and retrieval path usable.
        if (state != CubinAPIState::Missing)
            continue;
        artifact.setNull();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileCudaSource(compilers[0], SLANG_PTX, artifact)));
        diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(artifact);
        SLANG_CHECK_ABORT(diagnostics && SLANG_SUCCEEDED(diagnostics->getResult()));
        ComPtr<ISlangBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(artifact->loadBlob(ArtifactKeep::No, code.writeRef())));
        SLANG_CHECK(code->getBufferSize() > 0);
    }
}

SLANG_UNIT_TEST(cubinMissingTools, NoRenderApi)
{
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(global.writeRef())));
    global->setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVRTC, "missing-cubin-test-tool/nvrtc");
    SLANG_CHECK(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE) == SLANG_E_NOT_FOUND);
    SLANG_CHECK(global->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE_ASM) == SLANG_E_NOT_FOUND);

    global->setDownstreamCompilerPath(
        SLANG_PASS_THROUGH_NVDISASM,
        "missing-cubin-test-tool/nvdisasm");
    SLANG_CHECK(SLANG_FAILED(global->checkPassThroughSupport(SLANG_PASS_THROUGH_NVDISASM)));
}

SLANG_UNIT_TEST(cubinMissingDisassemblerOutput, CUDA)
{
    if (SLANG_FAILED(
            unitTestContext->slangGlobalSession->checkCompileTargetSupport(SLANG_CUDA_OBJECT_CODE)))
    {
        SLANG_IGNORE_TEST;
    }
    ComPtr<slang::IGlobalSession> global;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang::createGlobalSession(global.writeRef())));
    global->setDownstreamCompilerPath(
        SLANG_PASS_THROUGH_NVDISASM,
        "missing-cubin-test-tool/nvdisasm");

    // Exercise slangc's console output even when the test runner redirects its own stdout.
    StringBuilder consoleOutput;
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(global->createCompileRequest(request.writeRef())));
    SLANG_ALLOW_DEPRECATED_END
    request->setCommandLineCompilerMode();
    request->setWriter(
        SLANG_WRITER_CHANNEL_STD_OUTPUT,
        new StringWriter(&consoleOutput, WriterFlag::IsConsole));
    const char* args[] = {
        "tests/cuda/cubin.slang",
        "-entry",
        "computeMain",
        "-target",
        "cubin",
        "-capability",
        "cuda_sm_8_0",
    };
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(request->processCommandLineArguments(args, SLANG_COUNT_OF(args))));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->compile()));
    List<uint8_t> bytes;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(HexDumpUtil::parseWithMarkers(consoleOutput.getUnownedSlice(), bytes)));
    SLANG_CHECK(bytes.getCount() >= 4 && memcmp(bytes.getBuffer(), "\177ELF", 4) == 0);

    auto session = createCubinSession(global, SLANG_CUDA_OBJECT_CODE_ASM);
    auto entry = loadComputeEntryPoint(session);
    ComPtr<slang::IComponentType> linked;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(entry->link(linked.writeRef())));
    ComPtr<slang::IBlob> code, diagnostics;
    SLANG_CHECK(
        SLANG_FAILED(linked->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
    SLANG_CHECK(StringUtil::getSlice(diagnostics).indexOf(toSlice("nvdisasm")) >= 0);
}

SLANG_UNIT_TEST(cubinDisassemblerRejectsInvalidInput, CUDA)
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
