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
            return (void*)fakeCompileShrinking;
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
int compileAndReturnShippedWordCount(bool separateDebugInfo = false)
{
    gValidatedWordCount = -1;
    gValidatorCallCount = 0;
    gValidatedWordCounts.clear();
    gOptimizedWordCount = -1;

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
        SLANG_CHECK_ABORT(
            linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()) ==
            SLANG_OK);
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
