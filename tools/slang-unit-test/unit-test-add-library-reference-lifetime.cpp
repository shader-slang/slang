// unit-test-add-library-reference-lifetime.cpp

#include "core/slang-list.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;

// Checks that `spAddLibraryReference` does not depend on the caller's buffer once it has
// returned.
//
// The documented contract is that the caller may release `libData` as soon as the call
// returns. The implementation built a blob with `RawBlob::create`, which *copies*, and then
// asked the loader to parse the caller's pointer instead of the copy it retained, so every
// RIFF chunk pointer and fossil cursor referred into memory nothing kept alive.
//
// That is a use-after-free without any help from on-demand IR: AST declarations are already
// read lazily, so the library's declarations are decoded during semantic checking of the
// `import` below -- after this call returned. Leaving instruction bodies encoded adds a
// second route into the same bytes, later still, during linking and emit.
//
// So this test is not vacuous under either load mode, and it uses only the public C API --
// which is why it stays here, in the tool `slang-test` loads, and so runs against the shipped
// shared library on every configuration CI builds. The extra route that deferral opens is
// covered separately by `irDeferralDeclinesWhenTheBlobDoesNotBackTheSpans` in
// `slang-static-unit-test`, whose `Mismatched` case is exactly the blob shape this call had.
//
// The test poisons the caller's buffer in place rather than freeing it. Freed memory
// often still reads back intact, which would make this pass under the bug; overwriting is
// deterministic, and it also keeps the test itself free of any read-after-free. Under the
// bug the deferred decode reads the poison and the compile produces garbage, wrong
// results, or a crash. Under the fix the buffer is irrelevant the moment the call returns.
SLANG_UNIT_TEST(addLibraryReferenceDoesNotAliasCallerBuffer)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // Build the library the way a real caller does: compile to a module container, which
    // is what `addLibraryReference` parses. A bare `IModule::serialize()` blob is not that
    // format -- it has no container chunk -- and is rejected before any of this matters.
    List<uint8_t> libraryBytes;
    {
        ComPtr<slang::ICompileRequest> libRequest;
        SLANG_ALLOW_DEPRECATED_BEGIN
        SLANG_CHECK_ABORT(globalSession->createCompileRequest(libRequest.writeRef()) == SLANG_OK);
        SLANG_ALLOW_DEPRECATED_END

        // Both are needed for a module container to be produced: the format says what
        // shape the container takes, and `-emit-ir` is what puts the serialized IR in it.
        // With only the former, `maybeCreateContainer` finds no artifacts and
        // `getContainerCode` fails -- which is what slangc's `-o foo.slang-module` sets
        // together, and separately here because there is no output path to infer from.
        libRequest->setOutputContainerFormat(SLANG_CONTAINER_FORMAT_SLANG_MODULE);
        const char* const emitIrArg = "-emit-ir";
        SLANG_CHECK_ABORT(libRequest->processCommandLineArguments(&emitIrArg, 1) == SLANG_OK);

        const int libTu = libRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "uafLibrary");
        libRequest->addTranslationUnitSourceString(
            libTu,
            "uafLibrary.slang",
            "public float libScale(float v) { return v * 3.0f + 1.0f; }\n"
            "public float libTwice(float v) { return libScale(libScale(v)); }\n");

        const SlangResult libResult = libRequest->compile();
        if (SLANG_FAILED(libResult))
        {
            if (const char* diagnostics = libRequest->getDiagnosticOutput())
                getTestReporter()->message(TestMessageType::TestFailure, diagnostics);
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(libResult));

        ComPtr<slang::IBlob> container;
        SLANG_CHECK_ABORT(libRequest->getContainerCode(container.writeRef()) == SLANG_OK);
        SLANG_CHECK_ABORT(container && container->getBufferSize() > 0);

        libraryBytes.setCount(Index(container->getBufferSize()));
        ::memcpy(
            libraryBytes.getBuffer(),
            container->getBufferPointer(),
            container->getBufferSize());
    }

    // A buffer this test owns, standing in for a host's. Everything below turns on
    // whether the compiler still reads it after `addLibraryReference` returns.
    List<uint8_t> callerBuffer;
    callerBuffer.setCount(libraryBytes.getCount());
    ::memcpy(callerBuffer.getBuffer(), libraryBytes.getBuffer(), size_t(libraryBytes.getCount()));

    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK_ABORT(globalSession->createCompileRequest(request.writeRef()) == SLANG_OK);
    SLANG_ALLOW_DEPRECATED_END

    const int targetIndex = request->addCodeGenTarget(SLANG_HLSL);
    request->setTargetProfile(targetIndex, globalSession->findProfile("sm_5_0"));

    SLANG_CHECK_ABORT(
        spAddLibraryReference(
            request,
            nullptr,
            callerBuffer.getBuffer(),
            size_t(callerBuffer.getCount())) == SLANG_OK);

    // The call has returned, so per the contract these bytes are the caller's to do with
    // as it likes. Poison them.
    ::memset(callerBuffer.getBuffer(), 0xCD, size_t(callerBuffer.getCount()));

    // Now force the library's IR to be used. Linking and emit walk bodies that a deferred
    // load left encoded, so this is where a view into the poisoned buffer would be read.
    const int translationUnitIndex =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    request->addTranslationUnitSourceString(
        translationUnitIndex,
        "user.slang",
        "import uafLibrary;\n"
        "RWStructuredBuffer<float> gOut;\n"
        "[shader(\"compute\")]\n"
        "[numthreads(1,1,1)]\n"
        "void computeMain(uint3 tid : SV_DispatchThreadID)\n"
        "{ gOut[tid.x] = libTwice(2.0f); }\n");
    request->addEntryPoint(translationUnitIndex, "computeMain", SLANG_STAGE_COMPUTE);

    const SlangResult compileResult = request->compile();
    if (SLANG_FAILED(compileResult))
    {
        // Print the diagnostics: if this ever regresses, what the compiler said about the
        // poisoned bytes is the whole story.
        if (const char* diagnostics = request->getDiagnosticOutput())
            getTestReporter()->message(TestMessageType::TestFailure, diagnostics);
    }
    SLANG_CHECK(SLANG_SUCCEEDED(compileResult));

    // And the output has to be real, not merely non-failing -- a compile that silently
    // produced nothing would satisfy the check above while proving nothing.
    ComPtr<slang::IBlob> code;
    SLANG_CHECK(SLANG_SUCCEEDED(request->getEntryPointCodeBlob(0, targetIndex, code.writeRef())));
    SLANG_CHECK(code && code->getBufferSize() > 0);
}
