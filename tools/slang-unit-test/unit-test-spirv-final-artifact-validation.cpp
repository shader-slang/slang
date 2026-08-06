// unit-test-spirv-final-artifact-validation.cpp

// SPIR-V validation inspects the artifact the caller receives, not an intermediate one that a later
// pipeline step replaces.
//
// The optimize step and the debug-strip inside it each replace the artifact, so validating before
// them checks bytes the caller never gets. Validation is read-only, so when the module is
// acceptable either way the compiler's output and diagnostics are identical under both orderings;
// the only way to observe which module was inspected is to watch the validator itself.
//
// These tests install a fake `slang-glslang` whose validator records the module size it was handed
// and whose optimizer returns a module of a deliberately different size. Comparing the recorded
// size against the two candidates says which module validation received.

#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-glslang-compiler.h"
#include "core/slang-shared-library.h"
#include "scoped-env-var.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;
using SlangUnitTest::ScopedEnvVar;

namespace
{

// What the fake validator saw. Sizes are in 32-bit words, as `glslang_validateSPIRV` receives them.
// Every call is recorded, not just the last, because `-separate-debug-info` produces two shipped
// modules and the test needs to see which sizes were presented across both calls.
int gValidatedWordCount = -1;
int gValidatorCallCount = 0;
List<int> gValidatedWordCounts;

// The word count the fake optimizer produced, so a test can compare what was validated against
// what the optimize step actually returned rather than against a hardcoded expectation.
int gOptimizedWordCount = -1;

bool fakeValidateRecordingSize(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    gValidatedWordCount = contentsSize;
    gValidatedWordCounts.add(contentsSize);
    ++gValidatorCallCount;
    return true;
}

bool fakeDisassemble(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    return true;
}

// An optimizer that returns a strictly smaller module than it was given, so the pre-optimize and
// post-optimize sizes cannot be confused. Dropping trailing words would produce an invalid module,
// which is fine here: the fake validator accepts everything, and no real validation runs.
//
// The last word is dropped rather than a larger slice so the result stays a whole number of words
// and the emitted module keeps its header.
int fakeCompileShrinking(glslang_CompileRequest_1_3* request)
{
    if (!request || !request->outputFunc)
    {
        return 1;
    }
    const char* begin = (const char*)request->inputBegin;
    const char* end = (const char*)request->inputEnd;
    size_t byteCount = size_t(end - begin);
    if (byteCount >= sizeof(uint32_t))
    {
        byteCount -= sizeof(uint32_t);
    }
    gOptimizedWordCount = int(byteCount / sizeof(uint32_t));
    request->outputFunc(begin, byteCount, request->outputUserData);
    return 0;
}

// An optimizer that fails the way a real spirv-opt failure arrives: it writes a diagnostic and
// returns non-zero without producing any output. `GlslangDownstreamCompiler::compile` turns that
// into an artifact carrying an error diagnostic and **no blob**, while still returning `SLANG_OK`
// -- so the caller's success branch is taken and the artifact it installs has no SPIR-V in it.
int fakeCompileFailing(glslang_CompileRequest_1_3* request)
{
    if (request && request->diagnosticFunc)
    {
        const char* message = "fake optimizer failure\n";
        request->diagnosticFunc(message, strlen(message), request->diagnosticUserData);
    }
    return 1;
}

// Which optimizer the fake library exports. The failing variant exists to reach the path where the
// optimize step produces nothing, which is where validation has to fall back to the module the
// emitter produced.
enum class FakeOptimizer
{
    Shrinking,
    Failing,
};

FakeOptimizer gFakeOptimizer = FakeOptimizer::Shrinking;

// A shared library that exists only as a symbol table, so the test controls what the compiler's
// validate and optimize steps do.
class FakeGlslangLibrary : public RefObject, public ISlangSharedLibrary
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

        // Only the _1_3 compile symbol is exported: that is the signature `fakeCompileShrinking`
        // has, and the one the downstream compiler prefers. At least one compile entry point must
        // resolve or `init` rejects the library outright.
        if (symbol == "glslang_compile_1_3")
        {
            return gFakeOptimizer == FakeOptimizer::Failing ? (void*)fakeCompileFailing
                                                            : (void*)fakeCompileShrinking;
        }
        if (symbol == "glslang_disassembleSPIRV")
        {
            return (void*)fakeDisassemble;
        }
        if (symbol == "glslang_validateSPIRV")
        {
            return (void*)fakeValidateRecordingSize;
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
};

// Answers the versioned `slang-glslang-<version>` request with the fake library, and defers
// everything else to the default loader.
class FakeLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (UnownedStringSlice(path).indexOf(UnownedStringSlice("slang-glslang")) >= 0)
        {
            ComPtr<ISlangSharedLibrary> library(new FakeGlslangLibrary());
            *outLibrary = library.detach();
            return SLANG_OK;
        }
        return DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(path, outLibrary);
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

// Compile a trivial compute entry point to SPIR-V with validation forced on and the optimizer
// enabled, through the fake `slang-glslang`. Returns the size of the module the caller received,
// in 32-bit words, or -1 if no module came back.
//
// With `separateDebugInfo` the debug-strip step runs too, replacing the artifact a second time
// after the optimizer already replaced it once. That distinguishes validating the final module from
// validating merely the optimizer's output.
int compileAndReturnShippedWordCount(
    bool separateDebugInfo = false,
    FakeOptimizer optimizer = FakeOptimizer::Shrinking,
    SlangResult* outCodeResult = nullptr,
    String* outDiagnostics = nullptr)
{
    gValidatedWordCount = -1;
    gValidatorCallCount = 0;
    gValidatedWordCounts.clear();
    gOptimizedWordCount = -1;
    gFakeOptimizer = optimizer;

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader());
    globalSession->setSharedLibraryLoader(loader);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    // This test selects a non-zero optimization level so the optimize step runs.
    List<slang::CompilerOptionEntry> options;
    slang::CompilerOptionEntry optimization = {};
    optimization.name = slang::CompilerOptionName::Optimization;
    optimization.value.kind = slang::CompilerOptionValueKind::Int;
    optimization.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    options.add(optimization);
    if (separateDebugInfo)
    {
        slang::CompilerOptionEntry debugLevel = {};
        debugLevel.name = slang::CompilerOptionName::DebugInformation;
        debugLevel.value.kind = slang::CompilerOptionValueKind::Int;
        debugLevel.value.intValue0 = SLANG_DEBUG_INFO_LEVEL_STANDARD;
        options.add(debugLevel);

        slang::CompilerOptionEntry separate = {};
        separate.name = slang::CompilerOptionName::EmitSeparateDebug;
        separate.value.kind = slang::CompilerOptionValueKind::Int;
        separate.value.intValue0 = 1;
        options.add(separate);
    }
    targetDesc.compilerOptionEntries = options.getBuffer();
    targetDesc.compilerOptionEntryCount = (uint32_t)options.getCount();

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
        session->loadModuleFromSourceString("test", "test.slang", source, diagnostics.writeRef());
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
    {
        // Validation is only reachable through this environment variable; no API or command-line
        // option turns it on.
        ScopedEnvVar validateSpirv("SLANG_RUN_SPIRV_VALIDATION", "1");
        const SlangResult codeResult =
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
        if (outCodeResult)
        {
            *outCodeResult = codeResult;
        }
        else
        {
            SLANG_CHECK_ABORT(codeResult == SLANG_OK);
        }
    }

    // Reported separately from the result code: a failing compile that says nothing is a
    // different defect from a failing compile that explains itself, and only the diagnostics
    // text distinguishes them.
    if (outDiagnostics)
    {
        *outDiagnostics =
            diagnostics ? String((const char*)diagnostics->getBufferPointer()) : String();
    }

    return code ? int(code->getBufferSize() / sizeof(uint32_t)) : -1;
}

} // namespace

// The module handed to the validator must be the one the caller receives. The fake optimizer
// returns a smaller module than it was given, so validating before the optimize step records the
// larger pre-optimize size and validating after records the shipped size.
SLANG_UNIT_TEST(spirvValidationInspectsShippedArtifact)
{
    const int shippedWordCount = compileAndReturnShippedWordCount();

    // Guard the premise: without a validator call, or without the optimizer having changed the
    // size, the comparison below would hold for the wrong reason.
    SLANG_CHECK(gValidatorCallCount == 1);
    SLANG_CHECK(shippedWordCount > 0);
    SLANG_CHECK(gOptimizedWordCount > 0);

    // The shipped module is the optimizer's output, so the optimizer having shrunk it by exactly
    // one word means the pre-optimize module was one word larger. Both candidate sizes are
    // therefore known and distinct, which is what lets the final assertion discriminate: the old
    // ordering would have recorded `shippedWordCount + 1`.
    SLANG_CHECK(shippedWordCount == gOptimizedWordCount);

    SLANG_CHECK(gValidatedWordCount == shippedWordCount);
}

// The optimize step is not the only one that replaces the artifact: under `-separate-debug-info`
// the debug-strip runs afterwards and replaces it again. Validation must still see that final
// module, so this covers the second replacement path -- a validation call sitting between the
// optimize and the strip would satisfy the test above while failing this one.
SLANG_UNIT_TEST(spirvValidationInspectsStrippedArtifact)
{
    const int shippedWordCount = compileAndReturnShippedWordCount(true);

    SLANG_CHECK(shippedWordCount > 0);
    SLANG_CHECK(gOptimizedWordCount > 0);

    // The strip must actually have removed something, otherwise the stripped and unstripped modules
    // are the same size and this test cannot tell which one the validator was handed.
    SLANG_CHECK(shippedWordCount < gOptimizedWordCount);

    // Both modules the caller receives are validated: the stripped main artifact and the debug
    // companion written as `.dbg.spv`, which keeps the instructions the strip removed.
    SLANG_CHECK(gValidatorCallCount == 2);
    SLANG_CHECK(gValidatedWordCounts.getCount() == 2);
    SLANG_CHECK(gValidatedWordCounts[0] == shippedWordCount);
    SLANG_CHECK(gValidatedWordCounts[1] == gOptimizedWordCount);
}

// When the optimize step produces nothing, validation still runs -- on the module the emitter
// produced. `GlslangDownstreamCompiler::compile` reports an optimizer failure by attaching an error
// diagnostic and returning `SLANG_OK` with no blob, so the artifact installed for the caller
// carries no SPIR-V; validating it would report nothing useful. The compile fails either way, but a
// caller debugging an emitter bug needs the validator's account of the SPIR-V that was actually
// built, which is what ran before the validation call moved after the optimize step.
SLANG_UNIT_TEST(spirvValidationRunsOnPreOptimizeModuleWhenOptimizerFails)
{
    SlangResult codeResult = SLANG_OK;
    compileAndReturnShippedWordCount(false, FakeOptimizer::Failing, &codeResult);

    // The optimizer failed, so no module ships.
    SLANG_CHECK(SLANG_FAILED(codeResult));

    // The point of the test: validation was still invoked, and on a real module rather than the
    // blob-less artifact the failed optimize step installed.
    SLANG_CHECK(gValidatorCallCount == 1);
    SLANG_CHECK(gValidatedWordCount > 0);
}

// The same optimizer failure as the test above, but with `-separate-debug-info` on. That mode runs
// the debug-strip inside the optimize block, and the strip loads the optimizer's output -- which on
// this path carries no blob, so the strip fails and returns from `createArtifactFromIR` before
// either the downstream diagnostics or the validation call is reached. A compile that fails without
// saying why leaves the caller nothing to act on, so this checks the diagnostic survives rather
// than just the result code.
SLANG_UNIT_TEST(spirvValidationReportsOptimizerFailureUnderSeparateDebugInfo)
{
    SlangResult codeResult = SLANG_OK;
    String diagnostics;
    compileAndReturnShippedWordCount(true, FakeOptimizer::Failing, &codeResult, &diagnostics);

    SLANG_CHECK(SLANG_FAILED(codeResult));

    // The premise: without the fake optimizer's message reaching the sink, this path reports a bare
    // failure code and the caller cannot tell an optimizer failure from any other.
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("fake optimizer failure")) >= 0);
}

// A struct with a pointer to its own type makes the emitter describe the debug composite before its
// members exist, and it does that with `OpExtInstWithForwardRefsKHR` rather than `OpExtInst`. The
// debug-strip has to treat that opcode as a debug instruction too: keeping the instruction while
// stripping the `OpString` and `DebugSource` it references leaves ids in the stripped module with
// nothing defining them.
//
// This inspects the shipped module's opcodes rather than relying on SPIR-V validation, because
// validation only runs when `SLANG_RUN_SPIRV_VALIDATION` is set in the environment and the main
// test suite does not set it -- a test that checked for the validation error would pass whether or
// not the strip was correct.
SLANG_UNIT_TEST(spirvStripRemovesForwardReferencedDebugInstructions)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    List<slang::CompilerOptionEntry> options;
    slang::CompilerOptionEntry debugLevel = {};
    debugLevel.name = slang::CompilerOptionName::DebugInformation;
    debugLevel.value.kind = slang::CompilerOptionValueKind::Int;
    debugLevel.value.intValue0 = SLANG_DEBUG_INFO_LEVEL_STANDARD;
    options.add(debugLevel);

    slang::CompilerOptionEntry separate = {};
    separate.name = slang::CompilerOptionName::EmitSeparateDebug;
    separate.value.kind = slang::CompilerOptionValueKind::Int;
    separate.value.intValue0 = 1;
    options.add(separate);

    targetDesc.compilerOptionEntries = options.getBuffer();
    targetDesc.compilerOptionEntryCount = (uint32_t)options.getCount();

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // The pointer back to `Node` is what forces the forward reference.
    const char* source = R"SLANG(
        struct Node
        {
            int value;
            Node* next;
        };

        RWStructuredBuffer<int> outputBuffer;

        [numthreads(1, 1, 1)]
        void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
        {
            Node node;
            node.value = 7;
            node.next = nullptr;
            outputBuffer[0] = node.value;
        }
    )SLANG";

    ComPtr<slang::IBlob> diagnostics;
    auto module =
        session->loadModuleFromSourceString("test", "test.slang", source, diagnostics.writeRef());
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
    SLANG_CHECK_ABORT(
        linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);
    SLANG_CHECK_ABORT(code != nullptr);

    const auto* words = (const uint32_t*)code->getBufferPointer();
    const size_t wordCount = code->getBufferSize() / sizeof(uint32_t);
    // A SPIR-V module opens with a five-word header; instructions begin after it.
    const size_t headerWordCount = 5;
    SLANG_CHECK_ABORT(wordCount > headerWordCount);

    // Opcode numbers rather than the `Spv*` enums, which this test's includes do not reach.
    const uint32_t opExtInstWithForwardRefsKHR = 4433;
    const uint32_t opString = 7;

    // Walk the instruction stream, counting what the strip should and should not have left behind.
    int forwardRefCount = 0;
    int debugStringCount = 0;
    for (size_t i = headerWordCount; i < wordCount;)
    {
        const uint32_t instWordCount = words[i] >> 16;
        const uint32_t opCode = words[i] & 0xFFFF;
        if (instWordCount == 0)
        {
            break;
        }
        if (opCode == opExtInstWithForwardRefsKHR)
        {
            ++forwardRefCount;
        }
        if (opCode == opString)
        {
            ++debugStringCount;
        }
        i += instWordCount;
    }

    // Guard the premise: the strip only removes the debug strings this instruction would have
    // referenced, so a module that never had them cannot show the defect either way.
    SLANG_CHECK(debugStringCount <= 1);

    // The point of the test. Before the strip recognized this opcode, the instruction survived
    // while the ids it references did not.
    SLANG_CHECK(forwardRefCount == 0);
}
