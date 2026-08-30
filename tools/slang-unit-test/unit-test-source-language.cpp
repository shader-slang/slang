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
    // language as the command-line option. Deliberately pass both an explicit Slang language and a
    // `.slang` path so this GLSL-only syntax cannot compile unless that normalization takes effect.
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
