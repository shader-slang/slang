// unit-test-validate-with-downstream-compiler.cpp

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// `CompilerOptionName::ValidateWithDownstreamCompiler` (the `-validate-with-downstream-compiler`
// option) makes the compiler run the downstream compiler or validator of the target on the code
// that it emits, and fail the compile if the tool rejects that code.
//
// A `.slang` test cannot check that invalid output fails: slang-test removes the option from a
// `SIMPLE` test when the machine does not have the tool, so such a test would fail on that
// machine. This test checks which tools are available itself, and skips the targets whose tool is
// missing.

namespace
{

struct ValidationCase
{
    const char* name;
    SlangCompileTarget target;
    // The tool that validates the emitted code. The default shader model of HLSL is 5.1, so the
    // HLSL case needs FXC and not DXC.
    SlangPassThrough tool;
    // The target intrinsic that the invalid source calls, so that the emitted code has text that no
    // downstream compiler accepts.
    const char* invalidIntrinsicTarget;
};

// Valid output for every target: a compute shader that writes a constant.
static const char* kValidSource = R"(
    RWStructuredBuffer<float> outputBuffer;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uint3 tid : SV_DispatchThreadID)
    {
        outputBuffer[tid.x] = 1.0;
    }
)";

// SPIR-V assembly that the Slang assembler accepts, but that the SPIR-V validator rejects: the
// operand of `OpConvertSToF` must be an integer, and here it is a float.
static const char* kInvalidSpirvSource = R"(
    RWStructuredBuffer<float> outputBuffer;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uint3 tid : SV_DispatchThreadID)
    {
        float f = 1.0;
        float r = spirv_asm {
            result : $$float = OpConvertSToF $f
        };
        outputBuffer[tid.x] = r;
    }
)";

static String makeInvalidSource(const char* intrinsicTarget)
{
    StringBuilder sb;
    sb << "__target_intrinsic(" << intrinsicTarget << ", \"@@this is not valid code@@($0)\")\n"
       << "float invalidFunction(float x);\n"
       << R"(
    RWStructuredBuffer<float> outputBuffer;

    [shader("compute")]
    [numthreads(1, 1, 1)]
    void computeMain(uint3 tid : SV_DispatchThreadID)
    {
        outputBuffer[tid.x] = invalidFunction(1.0);
    }
)";
    return sb.produceString();
}

// Compiles the `computeMain` entry point of `source` for `target` and returns the result of
// `getEntryPointCode`, with the diagnostics in `outDiagnostics`.
static SlangResult compileEntryPoint(
    slang::IGlobalSession* globalSession,
    SlangCompileTarget target,
    const char* source,
    bool validateWithDownstreamCompiler,
    bool skipSpirvValidation,
    String& outDiagnostics)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = target;

    List<slang::CompilerOptionEntry> options;
    {
        // The SPIR-V optimizer can hang on SPIR-V that the validator rejects, so the test does not
        // optimize.
        slang::CompilerOptionEntry entry;
        entry.name = slang::CompilerOptionName::Optimization;
        entry.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_NONE;
        options.add(entry);
    }
    if (validateWithDownstreamCompiler)
    {
        slang::CompilerOptionEntry entry;
        entry.name = slang::CompilerOptionName::ValidateWithDownstreamCompiler;
        entry.value.intValue0 = 1;
        options.add(entry);
    }
    if (skipSpirvValidation)
    {
        slang::CompilerOptionEntry entry;
        entry.name = slang::CompilerOptionName::SkipSPIRVValidation;
        entry.value.intValue0 = 1;
        options.add(entry);
    }

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.compilerOptionEntries = options.getBuffer();
    sessionDesc.compilerOptionEntryCount = (uint32_t)options.getCount();

    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, session.writeRef()));

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = session->loadModuleFromSourceString(
        "validateTestModule",
        "validateTestModule.slang",
        source,
        diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("computeMain", entryPoint.writeRef()));

    ComPtr<slang::IComponentType> composite;
    slang::IComponentType* components[] = {module, entryPoint};
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        2,
        composite.writeRef(),
        diagnostics.writeRef()));

    ComPtr<slang::IBlob> code;
    diagnostics = nullptr;
    const SlangResult result =
        composite->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());
    outDiagnostics = diagnostics ? String((const char*)diagnostics->getBufferPointer()) : String();
    return result;
}

// Compiles with `compileEntryPoint` and checks that the compile succeeds or fails as expected. The
// name of the case and the diagnostics go to stderr when it does not, because `SLANG_CHECK` shows
// only the expression.
static bool checkCompile(
    const char* caseName,
    bool expectSuccess,
    slang::IGlobalSession* globalSession,
    SlangCompileTarget target,
    const char* source,
    bool validateWithDownstreamCompiler,
    bool skipSpirvValidation = false)
{
    String diagnostics;
    const SlangResult result = compileEntryPoint(
        globalSession,
        target,
        source,
        validateWithDownstreamCompiler,
        skipSpirvValidation,
        diagnostics);
    const bool matches = SLANG_SUCCEEDED(result) == expectSuccess;
    if (!matches)
    {
        fprintf(
            stderr,
            "%s: expected the compile to %s (validate=%d, skip-spirv-validation=%d)\n%s\n",
            caseName,
            expectSuccess ? "succeed" : "fail",
            int(validateWithDownstreamCompiler),
            int(skipSpirvValidation),
            diagnostics.getBuffer());
        fflush(stderr);
    }
    return matches;
}

} // namespace

SLANG_UNIT_TEST(ValidateWithDownstreamCompiler)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    static const ValidationCase cases[] = {
        {"hlsl", SLANG_HLSL, SLANG_PASS_THROUGH_FXC, "hlsl"},
        {"glsl", SLANG_GLSL, SLANG_PASS_THROUGH_GLSLANG, "glsl"},
        {"metal", SLANG_METAL, SLANG_PASS_THROUGH_METAL, "metal"},
        {"wgsl", SLANG_WGSL, SLANG_PASS_THROUGH_TINT, "wgsl"},
        {"cuda", SLANG_CUDA_SOURCE, SLANG_PASS_THROUGH_NVRTC, "cuda"},
        {"cpp", SLANG_CPP_SOURCE, SLANG_PASS_THROUGH_GENERIC_C_CPP, "cpp"},
    };

    for (const auto& testCase : cases)
    {
        if (SLANG_FAILED(globalSession->checkPassThroughSupport(testCase.tool)))
        {
            fprintf(stderr, "skipped %s: the downstream tool is not available\n", testCase.name);
            continue;
        }

        // NVRTC can be found but not usable, for example when its builtins library is missing. The
        // test then skips the case, as it does for a tool that is not found.
        if (testCase.target == SLANG_CUDA_SOURCE)
        {
            String diagnostics;
            if (SLANG_FAILED(compileEntryPoint(
                    globalSession,
                    SLANG_PTX,
                    kValidSource,
                    false,
                    false,
                    diagnostics)))
            {
                fprintf(stderr, "skipped %s: the downstream tool does not work\n", testCase.name);
                continue;
            }
        }

        // Valid output passes, with and without the option.
        SLANG_CHECK(
            checkCompile(testCase.name, true, globalSession, testCase.target, kValidSource, false));
        SLANG_CHECK(
            checkCompile(testCase.name, true, globalSession, testCase.target, kValidSource, true));

        // Invalid output passes without the option, because the compiler only writes the text,
        // and fails with the option.
        const String invalidSource = makeInvalidSource(testCase.invalidIntrinsicTarget);
        SLANG_CHECK(checkCompile(
            testCase.name,
            true,
            globalSession,
            testCase.target,
            invalidSource.getBuffer(),
            false));
        SLANG_CHECK(checkCompile(
            testCase.name,
            false,
            globalSession,
            testCase.target,
            invalidSource.getBuffer(),
            true));
    }

    // SPIR-V is emitted directly, and the validator runs without `SLANG_RUN_SPIRV_VALIDATION`.
    if (SLANG_SUCCEEDED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_GLSLANG)))
    {
        SLANG_CHECK(checkCompile("spirv", true, globalSession, SLANG_SPIRV, kValidSource, true));
        SLANG_CHECK(
            checkCompile("spirv", false, globalSession, SLANG_SPIRV, kInvalidSpirvSource, true));

        // `SkipSPIRVValidation` has priority over the new option.
        SLANG_CHECK(checkCompile(
            "spirv",
            true,
            globalSession,
            SLANG_SPIRV,
            kInvalidSpirvSource,
            true,
            true));
    }
    else
    {
        fprintf(stderr, "skipped spirv: the SPIR-V validator is not available\n");
    }

    // A target without a downstream tool is an error, and not a silent success.
    {
        String diagnostics;
        SLANG_CHECK(SLANG_FAILED(compileEntryPoint(
            globalSession,
            SLANG_CPP_HEADER,
            kValidSource,
            true,
            false,
            diagnostics)));
        SLANG_CHECK(diagnostics.indexOf("52015") != Index(-1));
    }
}
