// unit-test-experimental-module-import.cpp

// Every module-discovery path must enforce the experimental-feature gate at the common
// findOrImportModule boundary. Direct source APIs may define an experimental module without the
// feature, but loading it by name or importing it into a consumer requires explicit opt-in. The
// enabled and ordinary-module cases must continue to import successfully.

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;

static const char kExperimentalDependencySource[] = R"(
    [ExperimentalModule]
    module ExperimentalDependency;
    public struct Marker {}
)";

static const char kOrdinaryDependencySource[] = R"(
    module OrdinaryDependency;
    public struct Marker {}
)";

// Reports whether a diagnostic blob contains the experimental-feature import error.
static bool _diagnosticsRequireExperimentalFeature(ISlangBlob* diagnostics)
{
    if (!diagnostics)
        return false;

    auto text = UnownedStringSlice(
        (const char*)diagnostics->getBufferPointer(),
        diagnostics->getBufferSize());
    return text.indexOf(toSlice("need to enable '-experimental-feature'")) != -1;
}

// Creates a file system whose dependency can be reached only through source-file lookup.
static ComPtr<ISlangMutableFileSystem> _createExperimentalDependencyFileSystem()
{
    ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(fileSystem->saveFile(
        "ExperimentalDependency.slang",
        kExperimentalDependencySource,
        strlen(kExperimentalDependencySource))));
    return fileSystem;
}

// Builds one serialized module through the public API so precompiled import tests share the same
// producer and differ only in the source-level contract under test.
static ComPtr<ISlangBlob> _serializeModule(
    slang::IGlobalSession* globalSession,
    const char* moduleName,
    const char* source)
{
    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    String modulePath = String(moduleName) + ".slang";
    ComPtr<ISlangBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        moduleName,
        modulePath.getBuffer(),
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    SLANG_CHECK_ABORT(!diagnostics || diagnostics->getBufferSize() == 0);

    ComPtr<ISlangBlob> serializedModule;
    SLANG_CHECK_ABORT(module->serialize(serializedModule.writeRef()) == SLANG_OK);
    return serializedModule;
}

// ISession::loadModule loads a module as an import would, so source-file discovery through this
// API must enforce the gate too. This differs from loadModuleFromSourceString, which defines the
// module supplied by the caller.
SLANG_UNIT_TEST(loadModuleByNameRequiresExperimentalFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto fileSystem = _createExperimentalDependencyFileSystem();
    slang::SessionDesc sessionDesc = {};
    sessionDesc.fileSystem = fileSystem;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<ISlangBlob> diagnostics;
    auto experimentalModule = session->loadModule("ExperimentalDependency", diagnostics.writeRef());
    SLANG_CHECK(experimentalModule == nullptr);
    SLANG_CHECK(_diagnosticsRequireExperimentalFeature(diagnostics));
}

// Importing a module must enforce its experimental gate even when another API call has already
// inserted that module into the session cache. The explicit loadModuleFromSourceString API may
// define an experimental library; each consumer still needs to opt in before importing it.
SLANG_UNIT_TEST(cachedExperimentalModuleStillRequiresFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<ISlangBlob> diagnostics;
    auto experimentalModule = session->loadModuleFromSourceString(
        "ExperimentalDependency",
        "ExperimentalDependency.slang",
        kExperimentalDependencySource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(experimentalModule != nullptr);

    // Compile two independent consumers so the second iteration proves that every import request
    // is gated even after the first rejection leaves the dependency cached in the session.
    for (Index i = 0; i < 2; ++i)
    {
        diagnostics.setNull();
        String consumerName = "ExperimentalConsumer" + String(i);
        String consumerPath = consumerName + ".slang";
        auto consumer = session->loadModuleFromSourceString(
            consumerName.getBuffer(),
            consumerPath.getBuffer(),
            "import ExperimentalDependency;",
            diagnostics.writeRef());
        SLANG_CHECK(consumer == nullptr);
        SLANG_CHECK(_diagnosticsRequireExperimentalFeature(diagnostics));
    }
}

// Language-server checking deliberately skips IR generation. This proves that the checked AST
// attribute enforces the gate even when no IRExperimentalModuleDecoration can exist.
SLANG_UNIT_TEST(languageServerExperimentalModuleWithoutIRStillRequiresFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    // WorkspaceVersion selects this mode for language-server checking. Linkage::loadParsedModule
    // consequently checks source modules without lowering them to IR.
    auto linkage = static_cast<Linkage*>(session.get());
    linkage->contentAssistInfo.checkingMode = ContentAssistCheckingMode::General;

    ComPtr<ISlangBlob> diagnostics;
    auto experimentalModule = session->loadModuleFromSourceString(
        "ExperimentalDependency",
        "ExperimentalDependency.slang",
        kExperimentalDependencySource,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(experimentalModule != nullptr);
    SLANG_CHECK(!diagnostics || diagnostics->getBufferSize() == 0);
    SLANG_CHECK(static_cast<Module*>(experimentalModule)->getIRModule() == nullptr);

    diagnostics.setNull();
    auto consumer = session->loadModuleFromSourceString(
        "LanguageServerConsumer",
        "LanguageServerConsumer.slang",
        "import ExperimentalDependency;",
        diagnostics.writeRef());

    // Language-server checking returns a partial module after errors so editor features can keep
    // working. The diagnostic proves that the dependency itself was rejected.
    SLANG_CHECK(consumer != nullptr);
    SLANG_CHECK(_diagnosticsRequireExperimentalFeature(diagnostics));
}

// Translation units in one compile request are checked in order and recorded in a local module
// dictionary. Importing an earlier experimental translation unit must enforce the same gate as a
// session-cache or file-system lookup.
SLANG_UNIT_TEST(multiTranslationUnitExperimentalModuleStillRequiresFeature)
{
    ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    SLANG_CHECK(SLANG_SUCCEEDED(
        unitTestContext->slangGlobalSession->createCompileRequest(request.writeRef())));

    int dependencyTranslationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "ExperimentalDependency");
    request->addTranslationUnitSourceString(
        dependencyTranslationUnit,
        "ExperimentalDependency.slang",
        kExperimentalDependencySource);

    int consumerTranslationUnit =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "MultiTranslationUnitConsumer");
    request->addTranslationUnitSourceString(
        consumerTranslationUnit,
        "MultiTranslationUnitConsumer.slang",
        "import ExperimentalDependency;");
    request->setCompileFlags(SLANG_COMPILE_FLAG_NO_CODEGEN);

    SlangResult result = request->compile();
    SLANG_ALLOW_DEPRECATED_END

    SLANG_CHECK(SLANG_FAILED(result));
    UnownedStringSlice diagnostics(request->getDiagnosticOutput());
    SLANG_CHECK(diagnostics.indexOf(toSlice("need to enable '-experimental-feature'")) != -1);
}

// A dependency discovered through the file system is not in either module cache when its import
// begins. The source-load return path must apply the same gate as cached and serialized modules.
SLANG_UNIT_TEST(sourceExperimentalModuleStillRequiresFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto fileSystem = _createExperimentalDependencyFileSystem();
    slang::SessionDesc sessionDesc = {};
    sessionDesc.fileSystem = fileSystem;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<ISlangBlob> diagnostics;
    auto consumer = session->loadModuleFromSourceString(
        "SourceConsumer",
        "SourceConsumer.slang",
        "import ExperimentalDependency;",
        diagnostics.writeRef());

    SLANG_CHECK(consumer == nullptr);
    SLANG_CHECK(_diagnosticsRequireExperimentalFeature(diagnostics));
}

// Enabling the feature must preserve the common import path's success case. Use file-system
// discovery here as well so the positive and negative tests exercise the same source-module shape.
SLANG_UNIT_TEST(sourceExperimentalModuleImportsWithFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto fileSystem = _createExperimentalDependencyFileSystem();
    slang::CompilerOptionEntry experimentalFeatureOption = {};
    experimentalFeatureOption.name = slang::CompilerOptionName::ExperimentalFeature;
    experimentalFeatureOption.value.kind = slang::CompilerOptionValueKind::Int;
    experimentalFeatureOption.value.intValue0 = 1;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.fileSystem = fileSystem;
    sessionDesc.compilerOptionEntries = &experimentalFeatureOption;
    sessionDesc.compilerOptionEntryCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<ISlangBlob> diagnostics;
    auto consumer = session->loadModuleFromSourceString(
        "EnabledSourceConsumer",
        "EnabledSourceConsumer.slang",
        "import ExperimentalDependency; Marker marker;",
        diagnostics.writeRef());

    SLANG_CHECK(consumer != nullptr);
    SLANG_CHECK(!diagnostics || diagnostics->getBufferSize() == 0);
}

// An ordinary serialized module must restore a checked ModuleDecl and pass through the common
// validation boundary without being over-rejected.
SLANG_UNIT_TEST(precompiledOrdinaryModuleImportsWithoutFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto serializedModule =
        _serializeModule(globalSession, "OrdinaryDependency", kOrdinaryDependencySource);
    ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        fileSystem->saveFileBlob("OrdinaryDependency.slang-module", serializedModule)));

    slang::SessionDesc sessionDesc = {};
    sessionDesc.fileSystem = fileSystem;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<ISlangBlob> diagnostics;
    auto consumer = session->loadModuleFromSourceString(
        "OrdinaryPrecompiledConsumer",
        "OrdinaryPrecompiledConsumer.slang",
        "import OrdinaryDependency; Marker marker;",
        diagnostics.writeRef());

    SLANG_CHECK(consumer != nullptr);
    SLANG_CHECK(!diagnostics || diagnostics->getBufferSize() == 0);
}

// Serialized modules restore their checked AST before they become importable. Importing only the
// serialized blob verifies that its restored ExperimentalModule attribute still enforces the gate.
SLANG_UNIT_TEST(precompiledExperimentalModuleStillRequiresFeature)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    auto serializedModule =
        _serializeModule(globalSession, "ExperimentalDependency", kExperimentalDependencySource);
    ComPtr<ISlangMutableFileSystem> fileSystem(new MemoryFileSystem());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        fileSystem->saveFileBlob("ExperimentalDependency.slang-module", serializedModule)));

    slang::SessionDesc sessionDesc = {};
    sessionDesc.fileSystem = fileSystem;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<ISlangBlob> diagnostics;
    auto consumer = session->loadModuleFromSourceString(
        "PrecompiledConsumer",
        "PrecompiledConsumer.slang",
        "import ExperimentalDependency;",
        diagnostics.writeRef());

    SLANG_CHECK(consumer == nullptr);
    SLANG_CHECK(_diagnosticsRequireExperimentalFeature(diagnostics));
}
