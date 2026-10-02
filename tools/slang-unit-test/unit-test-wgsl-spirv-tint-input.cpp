// unit-test-wgsl-spirv-tint-input.cpp

#include "../../external/slang-tint-headers/slang-tint.h"
#include "core/slang-shared-library.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// The `wgsl-spirv` and `wgsl-spirv-asm` targets emit WGSL and hand it to Tint. That WGSL must be
// the same WGSL that `-target wgsl` produces, because all three targets share one reflection
// layout (see issue #13391). Separately, `wgsl-spirv` must reach Tint like `wgsl-spirv-asm` does,
// rather than fail with an "unhandled code generation target" internal error (see issue #8323).
//
// `slang-tint` is only fetched for Windows x64, so a `.slang` test cannot see the WGSL that Tint
// receives anywhere else. These tests install a fake loader through `setSharedLibraryLoader`, so a
// fake `slang-tint` records the WGSL it is given on every platform.

namespace
{

String gTintInputWgsl;
bool gFakeTintWasCalled = false;

// A minimal, well-formed SPIR-V header, which the fake Tint returns as its "compiled" output.
const uint32_t kFakeSpirv[] = {0x07230203, 0x00010000, 0x00080001, 1, 0};

int fakeTintCompile(tint_CompileRequest* request, tint_CompileResult* result)
{
    gFakeTintWasCalled = true;
    gTintInputWgsl = String(request->wgslCode, request->wgslCode + request->wgslCodeLength);
    result->buffer = (const uint8_t*)kFakeSpirv;
    result->bufferSize = sizeof(kFakeSpirv);
    result->error = nullptr;
    return 0;
}

void fakeTintFreeResult(tint_CompileResult* result)
{
    SLANG_UNUSED(result);
}

class FakeTintLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);
        if (symbol == "tint_compile")
            return (void*)fakeTintCompile;
        if (symbol == "tint_free_result")
            return (void*)fakeTintFreeResult;
        return nullptr;
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                guid == ISlangSharedLibrary::getTypeGuid())
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }
};

// Answers every `slang-tint` request, either with the fake library or with "not found", so a real
// `slang-tint` is never loaded. Every other library comes from the default loader, which keeps the
// rest of the toolchain (e.g. the SPIR-V disassembler) as it normally is.
class FakeTintLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeTintLoader(bool tintAvailable)
        : m_tintAvailable(tintAvailable)
    {
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (UnownedStringSlice(path).indexOf(UnownedStringSlice("slang-tint")) < 0)
            return DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(path, outLibrary);

        if (!m_tintAvailable)
            return SLANG_E_NOT_FOUND;

        ComPtr<ISlangSharedLibrary> library(new FakeTintLibrary());
        *outLibrary = library.detach();
        return SLANG_OK;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }

    bool m_tintAvailable;
};

// The repro from #13391. The constant buffer's scalar array must use the std140 element stride of
// 16 bytes, which the WGSL emitter spells as an array of `vec4<f32>`.
const char* kShaderSource = R"SLANG(
    cbuffer C { float a[2]; float b; }
    RWStructuredBuffer<float> o;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void main() { o[0] = a[1] + b; }
)SLANG";

const SlangCompileTarget kTargets[] = {SLANG_WGSL, SLANG_WGSL_SPIRV_ASM, SLANG_WGSL_SPIRV};
const SlangInt kWgslTargetIndex = 0;
const SlangInt kWgslSpirvAsmTargetIndex = 1;
const SlangInt kWgslSpirvTargetIndex = 2;

struct EntryPointCodeOutcome
{
    SlangResult result;
    String code;
    String diagnostics;
};

String blobToString(slang::IBlob* blob)
{
    if (!blob)
        return String();
    const char* begin = (const char*)blob->getBufferPointer();
    return String(begin, begin + blob->getBufferSize());
}

// A linked `kShaderSource`, together with the sessions it depends on. `IComponentType` does not
// keep its session alive, so the sessions are held here for as long as the program is used.
struct LinkedProgram
{
    ComPtr<slang::IGlobalSession> globalSession;
    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> program;
};

// All of `kTargets` share one session, so the WGSL comparison sees one program and one reflection
// layout, and the outputs can differ only because of the target.
void linkProgram(LinkedProgram& outLinked, bool tintAvailable)
{
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, outLinked.globalSession.writeRef()) ==
        SLANG_OK);

    ComPtr<ISlangSharedLibraryLoader> loader(new FakeTintLoader(tintAvailable));
    outLinked.globalSession->setSharedLibraryLoader(loader);

    slang::TargetDesc targetDescs[SLANG_COUNT_OF(kTargets)] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(kTargets); ++i)
        targetDescs[i].format = kTargets[i];

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = SLANG_COUNT_OF(kTargets);
    sessionDesc.targets = targetDescs;

    SLANG_CHECK_ABORT(
        outLinked.globalSession->createSession(sessionDesc, outLinked.session.writeRef()) ==
        SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = outLinked.session->loadModuleFromSourceString(
        "wgslSpirvTintInput",
        "wgsl-spirv-tint-input.slang",
        kShaderSource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(module->findEntryPointByName("main", entryPoint.writeRef()) == SLANG_OK);

    slang::IComponentType* components[] = {module, entryPoint};
    ComPtr<slang::IComponentType> program;
    SLANG_CHECK_ABORT(
        outLinked.session->createCompositeComponentType(
            components,
            SLANG_COUNT_OF(components),
            program.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    SLANG_CHECK_ABORT(
        program->link(outLinked.program.writeRef(), diagnostics.writeRef()) == SLANG_OK);
}

EntryPointCodeOutcome getEntryPointCode(slang::IComponentType* program, SlangInt targetIndex)
{
    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    EntryPointCodeOutcome outcome;
    outcome.result =
        program->getEntryPointCode(0, targetIndex, code.writeRef(), diagnostics.writeRef());
    outcome.code = blobToString(code);
    outcome.diagnostics = blobToString(diagnostics);
    return outcome;
}

String getTintInputWgsl(slang::IComponentType* program, SlangInt targetIndex)
{
    gFakeTintWasCalled = false;
    gTintInputWgsl = String();
    getEntryPointCode(program, targetIndex);
    SLANG_CHECK(gFakeTintWasCalled);
    return gTintInputWgsl;
}

} // namespace

SLANG_UNIT_TEST(wgslSpirvTintInputMatchesWgslTarget)
{
    LinkedProgram linked;
    linkProgram(linked, true);
    slang::IComponentType* program = linked.program;

    EntryPointCodeOutcome wgsl = getEntryPointCode(program, kWgslTargetIndex);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(wgsl.result));
    // Without this, two equally wrong layouts would still compare equal.
    SLANG_CHECK(wgsl.code.indexOf(UnownedStringSlice("array<vec4<f32>, i32(2)>")) >= 0);

    SLANG_CHECK(getTintInputWgsl(program, kWgslSpirvAsmTargetIndex) == wgsl.code);
    SLANG_CHECK(getTintInputWgsl(program, kWgslSpirvTargetIndex) == wgsl.code);
}

SLANG_UNIT_TEST(wgslSpirvTargetReturnsTintOutput)
{
    LinkedProgram linked;
    linkProgram(linked, true);
    slang::IComponentType* program = linked.program;

    gFakeTintWasCalled = false;
    EntryPointCodeOutcome spirv = getEntryPointCode(program, kWgslSpirvTargetIndex);
    SLANG_CHECK(SLANG_SUCCEEDED(spirv.result));
    SLANG_CHECK(gFakeTintWasCalled);
    SLANG_CHECK(
        spirv.code.getLength() == sizeof(kFakeSpirv) &&
        memcmp(spirv.code.getBuffer(), kFakeSpirv, sizeof(kFakeSpirv)) == 0);
}

// Without Tint, `wgsl-spirv` must fail by reporting the missing compiler, the "well-formed error
// result" that #8323 asks for, and not with an internal error.
SLANG_UNIT_TEST(wgslSpirvTargetWithoutTintReportsDiagnostic)
{
    LinkedProgram linked;
    linkProgram(linked, false);
    slang::IComponentType* program = linked.program;

    EntryPointCodeOutcome spirv = getEntryPointCode(program, kWgslSpirvTargetIndex);
    SLANG_CHECK(SLANG_FAILED(spirv.result));
    SLANG_CHECK(spirv.code.getLength() == 0);
    SLANG_CHECK(
        spirv.diagnostics.indexOf(
            UnownedStringSlice("failed to load downstream compiler 'tint'")) >= 0);
    SLANG_CHECK(
        spirv.diagnostics.indexOf(UnownedStringSlice("unhandled code generation target")) < 0);
}
