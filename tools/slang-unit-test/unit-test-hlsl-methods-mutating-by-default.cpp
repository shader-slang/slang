// The HLSL receiver-default option applies to unannotated methods in checked HLSL input.
// These tests exercise the public session option and the command-line-to-session conversion.

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(hlslMethodsMutatingByDefaultSessionOption)
{
    // A read-only body does not require a writable receiver by itself. Calling `read` on a const
    // local therefore distinguishes the method's checked receiver mode from its actual writes.
    // The macros let us verify that explicit mode spellings override the default on the same call.
    const char* source = R"(
        struct Counter
        {
            int value;

            RECEIVER_ATTRIBUTE
            int read() RECEIVER_CONST
            {
                return value;
            }
        };

        void checkReceiver()
        {
            const Counter counter = {1};
            counter.read();
        }
    )";

    // The global session must include GLSL support so that the GLSL case tests receiver policy
    // rather than failing because the dialect's builtin module is unavailable.
    SlangGlobalSessionDesc globalDesc = {};
    globalDesc.enableGLSL = true;
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession2(&globalDesc, globalSession.writeRef()) == SLANG_OK);

    // A case names a combination of input dialect, optional setting, and receiver spelling.
    struct TestCase
    {
        const char* description;
        SlangSourceLanguage language = SLANG_SOURCE_LANGUAGE_HLSL;
        bool hasOption = false;
        bool isEnabled = false;
        const char* sourcePath = "receiver-option.slang";
        const char* receiverAttribute = "";
        const char* receiverConst = "";
        bool shouldCompile = true;
    };
    const TestCase cases[] = {
        {.description = "HLSL with option omitted"},
        {.description = "HLSL with option disabled", .hasOption = true, .isEnabled = false},
        {.description = "HLSL with option enabled",
         .hasOption = true,
         .isEnabled = true,
         .shouldCompile = false},
        {.description = "Slang with option enabled",
         .language = SLANG_SOURCE_LANGUAGE_SLANG,
         .hasOption = true,
         .isEnabled = true,
         .sourcePath = "receiver-option.hlsl"},
        {.description = "GLSL with option enabled",
         .language = SLANG_SOURCE_LANGUAGE_GLSL,
         .hasOption = true,
         .isEnabled = true,
         .sourcePath = "receiver-option.hlsl"},
        {.description = "HLSL with option enabled and [nonmutating]",
         .hasOption = true,
         .isEnabled = true,
         .receiverAttribute = "[nonmutating]"},
        {.description = "HLSL with option enabled and trailing const",
         .hasOption = true,
         .isEnabled = true,
         .receiverConst = "const"},
        {.description = "HLSL with option disabled and [mutating]",
         .hasOption = true,
         .isEnabled = false,
         .receiverAttribute = "[mutating]",
         .shouldCompile = false},
    };

    for (const auto& testCase : cases)
    {
        // The deliberately mismatched extension makes the checked language observable: HLSL
        // with the option rejects the call in a `.slang` file, while Slang and GLSL accept the
        // same call in a `.hlsl` file. Each case gets a fresh session to avoid module-cache reuse.
        slang::CompilerOptionEntry options[2] = {};
        options[0].name = slang::CompilerOptionName::Language;
        options[0].value.kind = slang::CompilerOptionValueKind::Int;
        options[0].value.intValue0 = testCase.language;
        if (testCase.hasOption)
        {
            options[1].name = slang::CompilerOptionName::HLSLMethodsMutatingByDefault;
            options[1].value.kind = slang::CompilerOptionValueKind::Int;
            options[1].value.intValue0 = testCase.isEnabled ? 1 : 0;
        }

        slang::PreprocessorMacroDesc macros[] = {
            {"RECEIVER_ATTRIBUTE", testCase.receiverAttribute},
            {"RECEIVER_CONST", testCase.receiverConst},
        };
        slang::SessionDesc sessionDesc = {};
        sessionDesc.compilerOptionEntries = options;
        sessionDesc.compilerOptionEntryCount = testCase.hasOption ? 2 : 1;
        sessionDesc.preprocessorMacros = macros;
        sessionDesc.preprocessorMacroCount = SLANG_COUNT_OF(macros);
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        // When `loadModuleFromSourceString` rejects the program, its diagnostics must identify
        // this immutable-receiver call. Checking the reason prevents a parser or language-selection
        // error from satisfying a negative case.
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
            "receiverOption",
            testCase.sourcePath,
            source,
            diagnostics.writeRef()));
        String diagnosticText;
        if (diagnostics)
            diagnosticText = static_cast<const char*>(diagnostics->getBufferPointer());

        // We include the case description in both checks so that failures identify the dialect,
        // option setting, and annotation even when the compiler unexpectedly accepts the call.
        StringBuilder resultDescription;
        resultDescription << testCase.description << ":\n" << diagnosticText;
        bool didCompile = module != nullptr;
        getTestReporter()->addResultWithLocation(
            didCompile == testCase.shouldCompile,
            resultDescription.getBuffer(),
            __FILE__,
            __LINE__);
        if (!testCase.shouldCompile)
        {
            getTestReporter()->addResultWithLocation(
                diagnosticText.indexOf(
                    toSlice("mutating method 'read' cannot be called on an immutable value")) >= 0,
                resultDescription.getBuffer(),
                __FILE__,
                __LINE__);
        }
    }
}

SLANG_UNIT_TEST(hlslMethodsMutatingByDefaultCommandLineSessionOption)
{
    // Parsing `-fhlsl-methods-mutating-by-default` for a session must preserve the flag as a public
    // boolean option. The compilation cases above exercise the behavior of that same option entry.
    const char* args[] = {"-fhlsl-methods-mutating-by-default"};
    slang::SessionDesc sessionDesc = {};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->parseCommandLineArguments(
            SLANG_COUNT_OF(args),
            args,
            &sessionDesc,
            allocation.writeRef())));

    bool foundEnabledOption = false;
    for (SlangInt i = 0; i < sessionDesc.compilerOptionEntryCount; ++i)
    {
        const auto& option = sessionDesc.compilerOptionEntries[i];
        if (option.name != slang::CompilerOptionName::HLSLMethodsMutatingByDefault)
            continue;
        SLANG_CHECK(option.value.kind == slang::CompilerOptionValueKind::Int);
        foundEnabledOption = option.value.intValue0 == 1;
    }
    SLANG_CHECK(foundEnabledOption);
}
