// unit-test-source-language.cpp

#include "core/slang-memory-file-system.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

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
    slang::CompilerOptionEntry allowGLSLInputOption = {};
    allowGLSLInputOption.name = slang::CompilerOptionName::AllowGLSL;
    allowGLSLInputOption.value.kind = slang::CompilerOptionValueKind::Int;
    allowGLSLInputOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &allowGLSLInputOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));

    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(session->createCompileRequest(request.writeRef())));

    // The deprecated request-wide API must be reduced to the same effective per-translation-unit
    // language as the command-line option. It deliberately takes precedence over an explicit
    // non-GLSL selection: pass both an explicit Slang language and a `.slang` path so this
    // GLSL-only syntax cannot compile unless that normalization takes effect. The session also
    // carries `AllowGLSL`, exercising both entry paths into the idempotent normalization helper.
    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "forcedGlslLanguage");
    request->addTranslationUnitSourceString(
        translationUnit,
        "forced-glsl.slang",
        "layout(local_size_x = 1) in;\nvoid main() {}\n");
    request->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);
    request->setAllowGLSLInput(true);

    SlangResult result = request->compile();
    SLANG_CHECK(SLANG_SUCCEEDED(result));

    // Reusing a request may add translation units after an earlier compile. Normalization must run
    // again to reach the new input without repeating the request-wide deprecation diagnostic.
    int secondTranslationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "secondForcedGlslLanguage");
    request->addTranslationUnitSourceString(
        secondTranslationUnit,
        "second-forced-glsl.slang",
        "layout(local_size_x = 1) in;\nvoid secondMain() {}\n");
    result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_SUCCEEDED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    UnownedStringSlice warning("warning[E00117]");
    Index warningIndex = diagnostics.indexOf(warning);
    SLANG_CHECK(warningIndex >= 0);
    SLANG_CHECK(diagnostics.tail(warningIndex + warning.getLength()).indexOf(warning) < 0);
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("setAllowGLSLInput()")) >= 0);
}

SLANG_UNIT_TEST(sourceLanguageAllowGLSLDoesNotApplyToImportedModules)
{
    const char* importedSource = R"(
        #language slang 2026
        module importedSlangModule;
        public struct ImportedType { int value; }
    )";
    ComPtr<ISlangFileSystemExt> fileSystem = ComPtr<ISlangFileSystemExt>(new MemoryFileSystem());
    auto memoryFileSystem = static_cast<MemoryFileSystem*>(fileSystem.get());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(memoryFileSystem->saveFile(
        "imported-slang-module.slang",
        importedSource,
        strlen(importedSource))));

    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));
    request->setFileSystem(fileSystem);

    // `-allow-glsl` applies to the request's input translation units, not every source module that
    // happens to use the same linkage. The imported module's explicit Slang directive agrees with
    // its `.slang` path and must not be diagnosed as overriding an inherited GLSL selection.
    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "glslInputWithSlangImport");
    request->addTranslationUnitSourceString(
        translationUnit,
        "glsl-input-with-slang-import.slang",
        "import imported_slang_module;\n"
        "layout(local_size_x = 1) in;\n"
        "ImportedType value;\n"
        "void main() { value.value = 1; }\n");
    request->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);
    request->setAllowGLSLInput(true);

    SlangResult result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_SUCCEEDED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("warning[E00117]")) >= 0);
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("E00120")) < 0);
}

SLANG_UNIT_TEST(sourceLanguageReproPreservesRequestAllowGLSLBeforeCompile)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->enableReproCapture()));

    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "uncompiledAllowGLSLRepro");
    request->addTranslationUnitSourceString(
        translationUnit,
        "uncompiled-allow-glsl-repro.slang",
        "layout(local_size_x = 1) in;\nvoid main() {}\n");
    request->setAllowGLSLInput(true);

    // Saving before compilation exercises the request-local state directly: no normalization has
    // yet copied GLSL into the translation unit, so replay can succeed only if the repro stores the
    // compatibility bit itself.
    ComPtr<ISlangBlob> repro;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(request->saveRepro(repro.writeRef())));

    ComPtr<slang::ICompileRequest> replayRequest;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(replayRequest.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        replayRequest->loadRepro(nullptr, repro->getBufferPointer(), repro->getBufferSize())));
    replayRequest->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);
    SlangResult result = replayRequest->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_SUCCEEDED(result));
    UnownedStringSlice diagnostics(replayRequest->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("warning[E00117]")) >= 0);
}

SLANG_UNIT_TEST(sourceLanguageVersionAppliesToEveryPrimaryFile)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));

    // The directive is deliberately in the second file. Slang 202c rejects the legacy
    // `import glsl` in the first file, proving that preprocessing resolves the module's version
    // before either primary file is parsed.
    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "multiFileVersionedModule");
    request->addTranslationUnitSourceString(
        translationUnit,
        "first.slang",
        "module multiFileVersionedModule;\nimport glsl;\n");
    request->addTranslationUnitSourceString(
        translationUnit,
        "second.slang",
        "#language slang 202c\n");
    request->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);

    SlangResult result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_FAILED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("error[E00124]")) >= 0);
}

SLANG_UNIT_TEST(sourceLanguageRejectsConflictingPrimaryFileVersions)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));

    // Semantic checking has one module-wide language version, so accepting both directives would
    // make later behavior depend on source-file order. The first directive is retained only as a
    // deterministic recovery version after diagnosing the invalid request.
    int translationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "conflictingVersions");
    request->addTranslationUnitSourceString(
        translationUnit,
        "first.slang",
        "#language slang 2025\nmodule conflictingVersions;\n");
    request->addTranslationUnitSourceString(
        translationUnit,
        "second.slang",
        "#language slang 2026\n");
    request->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);

    SlangResult result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_FAILED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(UnownedStringSlice("error[E00126]")) >= 0);
}

SLANG_UNIT_TEST(sourceLanguageSessionAllowGLSLAppliesWithoutMutation)
{
    slang::CompilerOptionEntry allowGLSLInputOption = {};
    allowGLSLInputOption.name = slang::CompilerOptionName::AllowGLSL;
    allowGLSLInputOption.value.kind = slang::CompilerOptionValueKind::Int;
    allowGLSLInputOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &allowGLSLInputOption;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));

    // A SessionDesc option belongs to the session rather than one compile request, so it must
    // continue to select GLSL for direct module loads. Loading twice checks that normalization did
    // not consume the option from the shared linkage after the first module. The `AllowGLSL`
    // spelling remains deprecated, so each independent module-load request warns once.
    const char* source = "layout(local_size_x = 1) in;\nvoid main() {}\n";
    for (Index i = 0; i < 2; ++i)
    {
        String moduleName = String("sessionAllowGLSL") + String(i);
        String path = String("session-allow-glsl-") + String(i) + ".slang";
        ComPtr<slang::IBlob> diagnostics;
        ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
            moduleName.getBuffer(),
            path.getBuffer(),
            source,
            diagnostics.writeRef()));

        SLANG_CHECK(module != nullptr);
        SLANG_CHECK(diagnostics != nullptr);
        UnownedStringSlice diagnosticText(
            (const char*)diagnostics->getBufferPointer(),
            diagnostics->getBufferSize());
        SLANG_CHECK(diagnosticText.indexOf(UnownedStringSlice("warning[E00117]")) >= 0);
    }
}

SLANG_UNIT_TEST(sourceLanguageCommandLineAllowGLSLCanConfigureSession)
{
    const char* args[] = {"-allow-glsl"};
    slang::SessionDesc sessionDesc = {};
    ComPtr<ISlangUnknown> allocation;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->parseCommandLineArguments(
            SLANG_COUNT_OF(args),
            args,
            &sessionDesc,
            allocation.writeRef())));

    // Global command-line parsing produces a SessionDesc rather than compiling input files. The
    // request-local parser state must therefore be materialized as a session option in its output.
    bool foundAllowGLSL = false;
    for (SlangInt i = 0; i < sessionDesc.compilerOptionEntryCount; ++i)
    {
        const auto& entry = sessionDesc.compilerOptionEntries[i];
        if (entry.name == slang::CompilerOptionName::AllowGLSL)
        {
            foundAllowGLSL = entry.value.intValue0 != 0;
            break;
        }
    }
    SLANG_CHECK(foundAllowGLSL);
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

SLANG_UNIT_TEST(sourceLanguageModuleSourceStringUsesFileExtension)
{
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));

    // Module loading has no per-call language argument. With no session language option or source
    // directive, the `.vert` path must therefore select GLSL through the same translation-unit
    // extension resolution used for ordinary compile-request inputs. This layout declaration is
    // GLSL-only syntax, so successful loading makes the effective parser mode observable.
    const char* source = "layout(local_size_x = 1) in;\nvoid main() {}\n";
    ComPtr<slang::IBlob> diagnostics;
    ComPtr<slang::IModule> module(session->loadModuleFromSourceString(
        "extensionSelectedGlslModule",
        "extension-selected-module.vert",
        source,
        diagnostics.writeRef()));

    SLANG_CHECK(module != nullptr);
}
