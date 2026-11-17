// Repro capture preserves the option that selects the default HLSL method receiver mode.
// We check binary replay of an unchecked method and the flag written to an extracted command.

#include "core/slang-memory-file-system.h"
#include "core/slang-offset-container.h"
#include "core/slang-string-util.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-repro.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Loading a repro must restore the saved HLSL receiver default before checking method bodies.
// We replay snapshots saved before semantic checking into requests with the opposite setting.
SLANG_UNIT_TEST(hlslMutatingMethodDefaultSurvivesRepro)
{
    SLANG_ALLOW_DEPRECATED_BEGIN
    const bool optionValues[] = {false, true};
    const char* options[] = {"-fhlsl-methods-mutating-by-default"};
    for (bool isEnabled : optionValues)
    {
        // We save before compiling the shader, so semantic checking during replay must use
        // the saved HLSL receiver-default setting.
        ComPtr<slang::ICompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->enableReproCapture()));
        if (isEnabled)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                request->processCommandLineArguments(options, SLANG_COUNT_OF(options))));
        }

        // An enabled setting permits the write in `increment`. A disabled setting permits
        // calling `read` on a `const` receiver. Each program fails under the opposite setting.
        const char* shader =
            isEnabled ? "struct Counter\n"
                        "{\n"
                        "    int value;\n"
                        "    void increment() { value++; }\n"
                        "};\n"
                      : "struct Counter\n"
                        "{\n"
                        "    int value;\n"
                        "    int read() { return value; }\n"
                        "};\n"
                        "int readCounter(const Counter counter) { return counter.read(); }\n";
        int translationUnit =
            request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_HLSL, "hlslMutatingMethodRepro");
        request->addTranslationUnitSourceString(
            translationUnit,
            "hlsl-mutating-method-repro.hlsl",
            shader);
        ComPtr<ISlangBlob> repro;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->saveRepro(repro.writeRef())));

        // We start replay with the opposite setting to check restoration of true as well as
        // replacement of a previously enabled option by the saved false value.
        ComPtr<slang::ICompileRequest> replayRequest;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            unitTestContext->slangGlobalSession->createCompileRequest(replayRequest.writeRef())));
        if (!isEnabled)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                replayRequest->processCommandLineArguments(options, SLANG_COUNT_OF(options))));
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            replayRequest->loadRepro(nullptr, repro->getBufferPointer(), repro->getBufferSize())));
        replayRequest->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);
        SLANG_CHECK(SLANG_SUCCEEDED(replayRequest->compile()));
    }
    SLANG_ALLOW_DEPRECATED_END
}

// An extracted command needs the opt-in flag exactly when the saved receiver setting is enabled.
SLANG_UNIT_TEST(hlslMutatingMethodDefaultSurvivesReproExtraction)
{
    const bool optionValues[] = {false, true};
    for (bool isEnabled : optionValues)
    {
        // We use an empty request to isolate command extraction from file extraction and path
        // rewriting.
        OffsetContainer container;
        auto request = container.newObject<ReproUtil::RequestState>();
        container[request]->areHLSLMethodsMutatingByDefault = isEnabled;
        ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem);
        OffsetBase& base = container.asBase();
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(ReproUtil::extractFiles(base, base.asRaw(request), fileSystem)));

        // We check both values so extraction omits the flag when the saved setting is false.
        ComPtr<ISlangBlob> manifestBlob;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(fileSystem->loadFile("manifest.txt", manifestBlob.writeRef())));
        String manifest = StringUtil::getString(manifestBlob);
        bool hasOption =
            manifest.indexOf(UnownedStringSlice("-fhlsl-methods-mutating-by-default")) >= 0;
        SLANG_CHECK(hasOption == isEnabled);
    }
}
