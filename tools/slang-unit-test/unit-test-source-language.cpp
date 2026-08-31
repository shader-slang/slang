// unit-test-source-language.cpp

#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(sourceLanguageRejectsConflictingPrimaryFileExtensions)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));

    // Construct the API-only shape that slangc avoids by putting unlike source languages in
    // separate translation units. With no explicit language to resolve the disagreement, the
    // primary source-file extensions cannot define one coherent parser mode for this unit.
    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_UNKNOWN, "mixedSourceLanguages");
    request->addTranslationUnitSourceString(
        translationUnit,
        "first.slang",
        "void fromSlangFile() {}\n");
    request->addTranslationUnitSourceString(
        translationUnit,
        "second.vert",
        "void fromGlslFile() {}\n");

    SlangResult result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_FAILED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("error[E00122]")) >= 0);
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("first.slang")) >= 0);
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("second.vert")) >= 0);
}

SLANG_UNIT_TEST(sourceLanguageAllowGLSLNormalizesThroughDeprecatedAPI)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));

    // The deprecated request-wide API must be reduced to the same effective per-translation-unit
    // language as the command-line option. It deliberately takes precedence over an explicit
    // non-GLSL selection: pass both an explicit Slang language and a `.slang` path so this
    // GLSL-only syntax cannot compile unless that normalization takes effect.
    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "forcedGlslLanguage");
    request->addTranslationUnitSourceString(
        translationUnit,
        "forced-glsl.slang",
        "layout(local_size_x = 1) in;\nvoid main() {}\n");
    request->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);
    request->setAllowGLSLInput(true);

    SlangResult result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_SUCCEEDED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("warning[E00117]")) >= 0);
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("setAllowGLSLInput()")) >= 0);
}

SLANG_UNIT_TEST(sourceLanguageSessionOptionAppliesToModuleSourceString)
{
    // `loadModuleFromSourceString()` does not take a language parameter. A client such as
    // render-test therefore selects GLSL on the session and may still give its in-memory source a
    // `.slang` path. This GLSL-only buffer syntax must be parsed according to the explicit session
    // option rather than the path extension.
    slang::CompilerOptionEntry sourceLanguageOption = {};
    sourceLanguageOption.name = slang::CompilerOptionName::Language;
    sourceLanguageOption.value.kind = slang::CompilerOptionValueKind::Int;
    sourceLanguageOption.value.intValue0 = SLANG_SOURCE_LANGUAGE_GLSL;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &sourceLanguageOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));

    const char* source = R"(
        #version 450
        layout(local_size_x = 1) in;
        buffer OutputBuffer
        {
            uint value;
        } outputBuffer;
        void main() {}
    )";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "explicitGlslModule",
        "explicit-glsl.slang",
        source,
        diagnostics.writeRef()));

    SLANG_CHECK(module != nullptr);
}
