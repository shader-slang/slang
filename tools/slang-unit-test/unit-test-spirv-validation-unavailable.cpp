// unit-test-spirv-validation-unavailable.cpp

#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-glslang-compiler.h"
#include "core/slang-shared-library.h"
#include "scoped-env-var.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using SlangUnitTest::ScopedEnvVar;

// A `slang-glslang` that loads but does not export `glslang_validateSPIRV` must report the missing
// capability rather than report every shader as invalid.
//
// `init` tolerates a missing validator so a library that can still compile, optimize and link stays
// usable, which makes "loaded, but no validator" legitimate input that only the result code can
// distinguish from a rejected module.
//
// The condition is environmental rather than source-level, so no `.slang` test can reach it. These
// reproduce it in-process: `setSharedLibraryLoader` installs a fake loader that answers the
// versioned `slang-glslang-<version>` request with a fake library whose symbol table the test
// controls.

namespace
{

// Which validator the fake library exports, covering all three states the caller must tell apart.
enum class FakeValidatorState
{
    Absent,
    PresentRejecting,
    PresentAccepting,
};

// Set by the fake validators so the tests can tell "returned false" from "was never called".
bool gFakeValidatorWasCalled = false;

bool fakeValidateReturningFalse(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    gFakeValidatorWasCalled = true;
    return false;
}

bool fakeValidateReturningTrue(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    gFakeValidatorWasCalled = true;
    return true;
}

// Copy the input SPIR-V straight to the output, which is a valid response to the
// `GLSLANG_ACTION_OPTIMIZE_SPIRV` request that `slang-emit.cpp` issues after validation. Doing
// nothing keeps the fake honest: the bytes the compiler gets back are the bytes it produced.
int fakeCompileIdentity(glslang_CompileRequest_1_3* request)
{
    if (!request || !request->outputFunc)
    {
        return 1;
    }
    const char* begin = (const char*)request->inputBegin;
    const char* end = (const char*)request->inputEnd;
    request->outputFunc(begin, size_t(end - begin), request->outputUserData);
    return 0;
}

bool fakeDisassemble(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    return true;
}

// A shared library that exists only as a symbol table. `validatorState` decides whether
// `glslang_validateSPIRV` resolves and what it answers, which is the whole point of the test.
class FakeGlslangLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeGlslangLibrary(FakeValidatorState validatorState)
        : m_validatorState(validatorState)
    {
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);

        // At least one compile entry point must resolve, otherwise `init` rejects the library
        // outright and we would be testing the load-failure path instead of the missing-capability
        // one. Only the _1_3 symbol is exported, because that is the signature
        // `fakeCompileIdentity` actually has -- and the one `_invoke` prefers.
        if (symbol == "glslang_compile_1_3")
        {
            return (void*)fakeCompileIdentity;
        }
        if (symbol == "glslang_disassembleSPIRV")
        {
            return (void*)fakeDisassemble;
        }
        if (symbol == "glslang_validateSPIRV")
        {
            switch (m_validatorState)
            {
            case FakeValidatorState::PresentRejecting:
                return (void*)fakeValidateReturningFalse;
            case FakeValidatorState::PresentAccepting:
                return (void*)fakeValidateReturningTrue;
            default:
                return nullptr;
            }
        }
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

    FakeValidatorState m_validatorState;
};

class FakeLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeLoader(FakeValidatorState validatorState)
        : m_validatorState(validatorState)
    {
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        // On unix `locateGlslangSpirvDownstreamCompiler` probes pthread first; failing those keeps
        // the fake library bound to the one request we care about.
        UnownedStringSlice request(path);
        if (request.indexOf(UnownedStringSlice("slang-glslang")) < 0)
        {
            return SLANG_E_NOT_FOUND;
        }

        ComPtr<ISlangSharedLibrary> library(new FakeGlslangLibrary(m_validatorState));
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

    FakeValidatorState m_validatorState;
};

// Locate the SPIRV-opt compiler the way `slang-check.cpp` does, but against our fake loader, and
// hand back the `validate` implementation under test.
IDownstreamCompiler* getFakeSpirvOptCompiler(
    RefPtr<DownstreamCompilerSet>& ioSet,
    ISlangSharedLibraryLoader* loader)
{
    ioSet = new DownstreamCompilerSet;
    if (SLANG_FAILED(SpirvOptDownstreamCompilerUtil::locateCompilers(String(), loader, ioSet)))
    {
        return nullptr;
    }
    DownstreamCompilerDesc desc;
    desc.type = SLANG_PASS_THROUGH_SPIRV_OPT;
    return DownstreamCompilerUtil::findCompiler(
        ioSet,
        DownstreamCompilerUtil::MatchType::Newest,
        desc);
}

// A minimal, well-formed SPIR-V header. `validate` forwards it verbatim, so the contents only
// matter to the fake validator, which ignores them.
const uint32_t kSpirvHeader[] = {0x07230203, 0x00010000, 0x00080001, 1, 0};

} // namespace

// When the validator entry point is absent, `validate` must say "not available" rather than
// "failed", so its caller can tell the two apart.
SLANG_UNIT_TEST(spirvValidateReportsUnavailableWhenSymbolMissing)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeValidatorState::Absent));
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader);
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeValidatorWasCalled = false;
    const SlangResult result = compiler->validate(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader)));

    SLANG_CHECK(result == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK(!gFakeValidatorWasCalled);
}

// The control for the test above: a validator that is present and rejects the module must keep
// returning `SLANG_FAIL`, so an unconditional `SLANG_E_NOT_AVAILABLE` cannot satisfy both tests.
SLANG_UNIT_TEST(spirvValidateReportsFailWhenValidatorRejects)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeValidatorState::PresentRejecting));
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader);
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeValidatorWasCalled = false;
    const SlangResult result = compiler->validate(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader)));

    SLANG_CHECK(result == SLANG_FAIL);
    SLANG_CHECK(result != SLANG_E_NOT_AVAILABLE);
    // Proves the result came from the validator rather than from an early-out.
    SLANG_CHECK(gFakeValidatorWasCalled);
}

namespace
{

// What a compile against the fake library produced. Tests assert on the result and on whether any
// SPIR-V came back, not only on the diagnostic text: "was it reported" and "was unvalidated code
// still handed to the caller" are separate properties.
struct FakeGlslangCompileOutcome
{
    SlangResult codeResult;
    bool producedCode;
    String diagnostics;
};

// Compile a trivial valid compute shader to SPIR-V with validation enabled, against a fake
// `slang-glslang` whose validator is absent, present-and-rejecting, or present-and-accepting.
FakeGlslangCompileOutcome compileWithFakeGlslang(FakeValidatorState validatorState)
{

    ScopedEnvVar validateSpirv("SLANG_RUN_SPIRV_VALIDATION", "1");

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(validatorState));
    globalSession->setSharedLibraryLoader(loader);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* source = R"SLANG(
        RWStructuredBuffer<int> gOutputBuffer;

        [numthreads(4, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            gOutputBuffer[dispatchThreadID.x] = int(dispatchThreadID.x);
        }
    )SLANG";

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("m", "test.slang", source, diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* componentTypes[2] = {module, entryPoint.get()};
    ComPtr<slang::IComponentType> composedProgram;
    SLANG_CHECK_ABORT(
        session->createCompositeComponentType(
            componentTypes,
            2,
            composedProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK_ABORT(
        composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> code;
    diagnostics.setNull();
    FakeGlslangCompileOutcome outcome;
    outcome.codeResult =
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    outcome.producedCode = code && code->getBufferSize() != 0;

    if (diagnostics && diagnostics->getBufferSize())
    {
        outcome.diagnostics = String(
            (const char*)diagnostics->getBufferPointer(),
            (const char*)diagnostics->getBufferPointer() + diagnostics->getBufferSize());
    }
    return outcome;
}

} // namespace

// The consumer half: compiling a valid shader with the validator absent must report the missing
// capability rather than accuse the shader, and must do so as an error.
SLANG_UNIT_TEST(spirvValidationUnavailableDiagnosesMissingValidator)
{
    const FakeGlslangCompileOutcome outcome = compileWithFakeGlslang(FakeValidatorState::Absent);
    const UnownedStringSlice diagnosticSlice = outcome.diagnostics.getUnownedSlice();

    // Assert something was captured before asserting what it lacks: the negative checks below
    // would pass vacuously against an empty string, and would keep passing even if this test
    // stopped reaching the validation path at all.
    SLANG_CHECK(diagnosticSlice.getLength() != 0);

    // Requested validation that could not run must fail the compile and yield no SPIR-V -- handing
    // back an unchecked module is the outcome this diagnostic exists to prevent.
    SLANG_CHECK(SLANG_FAILED(outcome.codeResult));
    SLANG_CHECK(!outcome.producedCode);

    // Pin the severity, not just the wording: downgrading the diagnostic to a `warning(` leaves
    // every text assertion below passing.
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("error[E00115]")) >= 0);

    // The environment fault is named, and the shader is not blamed for it.
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("glslang_validateSPIRV")) >= 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("Validation of generated SPIR-V")) < 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("99999")) < 0);
}

// The other side of the caller's branch: when the validator is present and rejects the module, the
// validation-failure diagnostic must still be what the user sees, so routing every failure to the
// "unavailable" message cannot satisfy both this test and the one above.
SLANG_UNIT_TEST(spirvValidationRejectionStillDiagnosesValidationFailure)
{
    const FakeGlslangCompileOutcome outcome =
        compileWithFakeGlslang(FakeValidatorState::PresentRejecting);
    const UnownedStringSlice diagnosticSlice = outcome.diagnostics.getUnownedSlice();

    SLANG_CHECK(diagnosticSlice.getLength() != 0);
    SLANG_CHECK(SLANG_FAILED(outcome.codeResult));
    SLANG_CHECK(!outcome.producedCode);

    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("Validation of generated SPIR-V")) >= 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("glslang_validateSPIRV")) < 0);
}

// The success path, which the two failure tests above cannot cover: a validator that is present and
// accepts must compile cleanly, return SPIR-V, and say nothing.
SLANG_UNIT_TEST(spirvValidationAcceptingValidatorReportsNothing)
{
    gFakeValidatorWasCalled = false;
    const FakeGlslangCompileOutcome outcome =
        compileWithFakeGlslang(FakeValidatorState::PresentAccepting);
    const UnownedStringSlice diagnosticSlice = outcome.diagnostics.getUnownedSlice();

    // Proves the silence below comes from an accepting validator rather than from never reaching
    // it.
    SLANG_CHECK(gFakeValidatorWasCalled);
    SLANG_CHECK(SLANG_SUCCEEDED(outcome.codeResult));
    SLANG_CHECK(outcome.producedCode);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("E00115")) < 0);

    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("glslang_validateSPIRV")) < 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("Validation of generated SPIR-V")) < 0);
    SLANG_CHECK(diagnosticSlice.indexOf(UnownedStringSlice("115")) < 0);
}
