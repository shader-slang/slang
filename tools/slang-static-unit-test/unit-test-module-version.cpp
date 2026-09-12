// unit-test-module-version.cpp

#include "core/slang-memory-file-system.h"
#include "core/slang-riff.h"
#include "slang-com-ptr.h"
#include "slang-deprecated.h"
#include "slang.h"
#include "slang/slang-fossil.h"
#include "slang/slang-ir.h"
#include "slang/slang-serialize-container.h"
#include "slang/slang-serialize-ir.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

#include <limits>

using namespace Slang;

namespace
{

// IRModuleInfo uses its FIDDLE field order, while IRModule uses the serialized order defined by
// Fossilized_IRModule and IRSerialWriteContext::handleIRModule. The tests use these indices to
// mutate serialized bytes without exposing private implementation types to test code.
static const Index kSerializationVersionFieldIndex = 0; // IRModuleInfo::serializationVersion
static const Index kModuleFieldIndex = 2;               // IRModuleInfo::module
static const Index kModuleVersionFieldIndex = 1;        // IRModule::m_version

/// Appends one diagnostic callback message to a StringBuilder.
void collectDiagnostic(const char* message, void* userData)
{
    static_cast<StringBuilder*>(userData)->append(message);
}

/// Finds the mutable fossilized IRModuleInfo record in `container`.
Fossil::ValPtr<FossilizedRecordVal> getModuleInfoRecord(List<uint8_t>& container)
{
    auto rootChunk = RIFF::RootChunk::getFromBlob(container.getBuffer(), container.getCount());
    if (!rootChunk)
        return nullptr;

    auto moduleChunk = ModuleChunk::find(rootChunk);
    if (!moduleChunk)
        return nullptr;

    auto dataChunk = as<RIFF::DataChunk>(moduleChunk->findIR());
    if (!dataChunk)
        return nullptr;

    auto rootVal = Fossil::getRootValue(dataChunk->getPayload(), dataChunk->getPayloadSize());
    if (!rootVal || rootVal->getKind() != FossilizedValKind::Struct)
        return nullptr;

    return Fossil::cast<FossilizedRecordVal>(rootVal);
}

/// Finds the mutable serialization-format version field in `container`.
UInt64* getSerializationVersion(List<uint8_t>& container)
{
    auto moduleInfo = getModuleInfoRecord(container);
    if (!moduleInfo || moduleInfo->getFieldCount() <= kSerializationVersionFieldIndex)
        return nullptr;

    auto field = moduleInfo->getField(kSerializationVersionFieldIndex);
    if (field.getKind() != FossilizedValKind::UInt64)
        return nullptr;

    return static_cast<UInt64*>(field.getDataPtr());
}

/// Finds the mutable pointer to the serialized IRModule record in `container`.
Fossil::AnyValRef getModulePointerField(List<uint8_t>& container)
{
    auto moduleInfo = getModuleInfoRecord(container);
    if (!moduleInfo || moduleInfo->getFieldCount() <= kModuleFieldIndex)
        return Fossil::AnyValRef();

    auto field = moduleInfo->getField(kModuleFieldIndex);
    if (field.getKind() != FossilizedValKind::Ptr)
        return Fossil::AnyValRef();
    return field;
}

/// Finds the mutable semantic IR module version field in `container`.
UInt64* getModuleVersion(List<uint8_t>& container)
{
    auto moduleField = getModulePointerField(container);
    if (!moduleField.getDataPtr())
        return nullptr;

    auto modulePtr = Fossil::cast<FossilizedPtr<void>>(Fossil::getAddress(moduleField));
    auto moduleVal = modulePtr->getTargetValPtr();
    if (!moduleVal || moduleVal->getKind() != FossilizedValKind::Struct)
        return nullptr;

    auto module = Fossil::cast<FossilizedRecordVal>(moduleVal);
    if (module->getFieldCount() <= kModuleVersionFieldIndex)
        return nullptr;

    auto field = module->getField(kModuleVersionFieldIndex);
    if (field.getKind() != FossilizedValKind::UInt64)
        return nullptr;
    return static_cast<UInt64*>(field.getDataPtr());
}

/// Creates a SPIR-V session, optionally enabling binary freshness checks.
bool createSession(
    slang::IGlobalSession* globalSession,
    ISlangFileSystem* fileSystem,
    bool useFreshnessCheck,
    ComPtr<slang::ISession>& outSession)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;

    slang::CompilerOptionEntry option = {};
    option.name = slang::CompilerOptionName::UseUpToDateBinaryModule;
    option.value.kind = slang::CompilerOptionValueKind::Int;
    option.value.intValue0 = useFreshnessCheck;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    sessionDesc.fileSystem = fileSystem;
    if (useFreshnessCheck)
    {
        sessionDesc.compilerOptionEntryCount = 1;
        sessionDesc.compilerOptionEntries = &option;
    }
    return SLANG_SUCCEEDED(globalSession->createSession(sessionDesc, outSession.writeRef()));
}

/// Compiles and serializes one module into mutable container bytes.
bool serializeModule(
    UnitTestContext* unitTestContext,
    const char* moduleName,
    const char* source,
    List<uint8_t>& outContainer)
{
    ComPtr<slang::ISession> session;
    if (!createSession(unitTestContext->slangGlobalSession, nullptr, false, session))
        return false;

    String path = String(moduleName) + ".slang";
    ComPtr<ISlangBlob> diagnostics;
    ComPtr<slang::IModule> module;
    module = session->loadModuleFromSourceString(
        moduleName,
        path.getBuffer(),
        source,
        diagnostics.writeRef());
    if (!module)
        return false;

    ComPtr<ISlangBlob> blob;
    if (SLANG_FAILED(module->serialize(blob.writeRef())) || !blob)
        return false;

    outContainer.clear();
    outContainer.addRange(
        static_cast<const uint8_t*>(blob->getBufferPointer()),
        blob->getBufferSize());
    return true;
}

/// Compiles and serializes one module into a module-library container.
bool serializeModuleLibrary(
    UnitTestContext* unitTestContext,
    const char* moduleName,
    const char* source,
    List<uint8_t>& outContainer)
{
    ComPtr<slang::ISession> session;
    if (!createSession(unitTestContext->slangGlobalSession, nullptr, false, session))
        return false;

    ComPtr<SlangCompileRequest> request;
    if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
        return false;

    const char* emitIrArgs[] = {"-emit-ir"};
    if (SLANG_FAILED(request->processCommandLineArguments(emitIrArgs, 1)))
        return false;
    request->setOutputContainerFormat(SLANG_CONTAINER_FORMAT_SLANG_MODULE);
    const int translationUnitIndex =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, moduleName);
    String path = String(moduleName) + ".slang";
    request->addTranslationUnitSourceString(translationUnitIndex, path.getBuffer(), source);
    if (SLANG_FAILED(request->compile()))
        return false;

    ComPtr<ISlangBlob> blob;
    if (SLANG_FAILED(request->getContainerCode(blob.writeRef())) || !blob)
        return false;

    outContainer.clear();
    outContainer.addRange(
        static_cast<const uint8_t*>(blob->getBufferPointer()),
        blob->getBufferSize());
    return true;
}

/// Returns the diagnostic blob contents as a non-owning string slice.
UnownedStringSlice getDiagnosticText(ISlangBlob* diagnostics)
{
    if (!diagnostics)
        return UnownedStringSlice();
    return UnownedStringSlice(
        static_cast<const char*>(diagnostics->getBufferPointer()),
        diagnostics->getBufferSize());
}

/// Counts non-overlapping occurrences of `needle` in `text`.
Count countOccurrences(UnownedStringSlice text, UnownedStringSlice needle)
{
    Count count = 0;
    for (;;)
    {
        Index index = text.indexOf(needle);
        if (index < 0)
            return count;
        count++;
        text = text.tail(index + needle.getLength());
    }
}

/// Returns whether `module` contains a direct declaration named `name`.
bool moduleHasDirectDeclaration(slang::IModule* module, UnownedStringSlice name)
{
    auto moduleReflection = module->getModuleReflection();
    if (!moduleReflection)
        return false;

    for (auto child : moduleReflection->getChildren())
    {
        const char* childName = child->getName();
        if (childName && UnownedStringSlice(childName) == name)
            return true;
    }
    return false;
}

} // namespace

SLANG_UNIT_TEST(serializedModuleVersionValidation)
{
    const char* source = R"(
        module version_test;
        public int getValue() { return 1; }
    )";

    List<uint8_t> container;
    SLANG_CHECK_ABORT(serializeModule(unitTestContext, "version_test", source, container));
    SLANG_CHECK_ABORT(getModuleVersion(container));
    SLANG_CHECK_ABORT(getSerializationVersion(container));
    SLANG_CHECK(*getModuleVersion(container) == IRModule::k_maxSupportedModuleVersion);
    // IRModuleInfo is private to slang-serialize-ir.cpp, so this literal must track
    // IRModuleInfo::kSupportedSerializationVersion.
    SLANG_CHECK(*getSerializationVersion(container) == 2);

    {
        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));

        ComPtr<ISlangBlob> diagnostics;
        ComPtr<slang::IModule> loaded;
        loaded = slang_loadModuleFromIRBlob(
            session,
            "version_test_current",
            "version_test_current.slang-module",
            container.getBuffer(),
            container.getCount(),
            diagnostics.writeRef());
        SLANG_CHECK(loaded != nullptr);
        SLANG_CHECK(getDiagnosticText(diagnostics).getLength() == 0);
    }

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = IRModule::k_minSupportedModuleVersion;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));

        ComPtr<ISlangBlob> diagnostics;
        ComPtr<slang::IModule> loaded;
        loaded = slang_loadModuleFromIRBlob(
            session,
            "version_test_min",
            "version_test_min.slang-module",
            patched.getBuffer(),
            patched.getCount(),
            diagnostics.writeRef());
        SLANG_CHECK(loaded != nullptr);
        SLANG_CHECK(getDiagnosticText(diagnostics).getLength() == 0);
    }

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = IRModule::k_minSupportedModuleVersion - 1;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));

        ComPtr<ISlangBlob> diagnostics;
        SLANG_CHECK(
            slang_loadModuleFromIRBlob(
                session,
                "version_test_old",
                "version_test_old.slang-module",
                patched.getBuffer(),
                patched.getCount(),
                diagnostics.writeRef()) == nullptr);
        SLANG_CHECK(getDiagnosticText(diagnostics).indexOf(toSlice("error[E00117]")) >= 0);
    }

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = IRModule::k_maxSupportedModuleVersion + 1;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));

        ComPtr<ISlangBlob> diagnostics;
        auto loaded = slang_loadModuleFromIRBlob(
            session,
            "version_test_explicit",
            "version_test_explicit.slang-module",
            patched.getBuffer(),
            patched.getCount(),
            diagnostics.writeRef());
        SLANG_CHECK(loaded == nullptr);
        SLANG_CHECK(getDiagnosticText(diagnostics).indexOf(toSlice("error[E00117]")) >= 0);

        SlangInt version = 0;
        const char* compilerVersion = nullptr;
        const char* moduleName = nullptr;
        SLANG_CHECK(
            slang_loadModuleInfoFromIRBlob(
                session,
                patched.getBuffer(),
                patched.getCount(),
                version,
                compilerVersion,
                moduleName) == SLANG_OK);
        SLANG_CHECK(version == SlangInt(IRModule::k_maxSupportedModuleVersion + 1));
    }

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = (UInt64(1) << 32) + IRModule::k_minSupportedModuleVersion;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));
        ComPtr<ISlangBlob> diagnostics;
        SLANG_CHECK(
            slang_loadModuleFromIRBlob(
                session,
                "version_test_wide",
                "version_test_wide.slang-module",
                patched.getBuffer(),
                patched.getCount(),
                diagnostics.writeRef()) == nullptr);
        SLANG_CHECK(getDiagnosticText(diagnostics).indexOf(toSlice("error[E00117]")) >= 0);
    }

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = (std::numeric_limits<UInt64>::max)();

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));

        SlangInt version = 0;
        const char* compilerVersion = nullptr;
        const char* moduleName = nullptr;
        SLANG_CHECK(SLANG_FAILED(slang_loadModuleInfoFromIRBlob(
            session,
            patched.getBuffer(),
            patched.getCount(),
            version,
            compilerVersion,
            moduleName)));
    }

    {
        List<uint8_t> patched = container;
        *getSerializationVersion(patched) = 1;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));
        ComPtr<ISlangBlob> diagnostics;
        SLANG_CHECK(
            slang_loadModuleFromIRBlob(
                session,
                "version_test_format",
                "version_test_format.slang-module",
                patched.getBuffer(),
                patched.getCount(),
                diagnostics.writeRef()) == nullptr);
        SLANG_CHECK(getDiagnosticText(diagnostics).indexOf(toSlice("error[E00119]")) >= 0);

        SlangInt version = 0;
        const char* compilerVersion = nullptr;
        const char* moduleName = nullptr;
        SLANG_CHECK(SLANG_FAILED(slang_loadModuleInfoFromIRBlob(
            session,
            patched.getBuffer(),
            patched.getCount(),
            version,
            compilerVersion,
            moduleName)));
    }

    {
        List<uint8_t> patched = container;
        auto moduleField = getModulePointerField(patched);
        SLANG_CHECK_ABORT(moduleField.getDataPtr());
        memset(moduleField.getDataPtr(), 0, sizeof(FossilizedPtr<void>));

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));
        ComPtr<ISlangBlob> diagnostics;
        SLANG_CHECK(
            slang_loadModuleFromIRBlob(
                session,
                "version_test_null",
                "version_test_null.slang-module",
                patched.getBuffer(),
                patched.getCount(),
                diagnostics.writeRef()) == nullptr);

        SlangInt version = 0;
        const char* compilerVersion = nullptr;
        const char* moduleName = nullptr;
        SLANG_CHECK(SLANG_FAILED(slang_loadModuleInfoFromIRBlob(
            session,
            patched.getBuffer(),
            patched.getCount(),
            version,
            compilerVersion,
            moduleName)));
    }
}

SLANG_UNIT_TEST(serializedModuleVersionLibraryReference)
{
    const char* source = R"(
        module version_test_library;
        public int getValue() { return 1; }
    )";

    List<uint8_t> container;
    SLANG_CHECK_ABORT(
        serializeModuleLibrary(unitTestContext, "version_test_library", source, container));

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = IRModule::k_maxSupportedModuleVersion + 1;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));
        ComPtr<SlangCompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompileRequest(request.writeRef())));
        StringBuilder diagnostics;
        request->setDiagnosticCallback(collectDiagnostic, &diagnostics);

        SLANG_CHECK(SLANG_FAILED(spAddLibraryReference(
            request,
            "version_test_library.slang-module",
            patched.getBuffer(),
            patched.getCount())));
        SLANG_CHECK(diagnostics.getUnownedSlice().indexOf(toSlice("error[E00117]")) >= 0);
    }

    {
        List<uint8_t> patched = container;
        *getSerializationVersion(patched) = 1;

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, nullptr, false, session));
        ComPtr<SlangCompileRequest> request;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->createCompileRequest(request.writeRef())));
        StringBuilder diagnostics;
        request->setDiagnosticCallback(collectDiagnostic, &diagnostics);

        SLANG_CHECK(SLANG_FAILED(spAddLibraryReference(
            request,
            "version_test_library.slang-module",
            patched.getBuffer(),
            patched.getCount())));
        SLANG_CHECK(diagnostics.getUnownedSlice().indexOf(toSlice("error[E00119]")) >= 0);
    }
}

SLANG_UNIT_TEST(serializedModuleVersionImportFallback)
{
    const char* binarySource = R"(
        module future;
        public int binaryOnly() { return 1; }
    )";
    const char* sourceFallback = R"(
        module future;
        public int sourceOnly() { return 2; }
    )";

    List<uint8_t> container;
    SLANG_CHECK_ABORT(serializeModule(unitTestContext, "future", binarySource, container));

    ComPtr<ISlangMutableFileSystem> fileSystem =
        ComPtr<ISlangMutableFileSystem>(new MemoryFileSystem());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        fileSystem->saveFile("future.slang", sourceFallback, strlen(sourceFallback))));

    {
        List<uint8_t> patched = container;
        *getModuleVersion(patched) = IRModule::k_maxSupportedModuleVersion + 1;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            fileSystem->saveFile("future.slang-module", patched.getBuffer(), patched.getCount())));

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, fileSystem, true, session));
        ComPtr<ISlangBlob> diagnostics;
        ComPtr<slang::IModule> module;
        module = session->loadModule("future", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
        SLANG_CHECK(moduleHasDirectDeclaration(module, toSlice("sourceOnly")));
        SLANG_CHECK(!moduleHasDirectDeclaration(module, toSlice("binaryOnly")));

        auto text = getDiagnosticText(diagnostics);
        SLANG_CHECK(countOccurrences(text, toSlice("warning[E00118]")) == 1);
    }

    {
        List<uint8_t> patched = container;
        *getSerializationVersion(patched) = 1;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            fileSystem->saveFile("future.slang-module", patched.getBuffer(), patched.getCount())));

        ComPtr<slang::ISession> session;
        SLANG_CHECK_ABORT(
            createSession(unitTestContext->slangGlobalSession, fileSystem, true, session));
        ComPtr<ISlangBlob> diagnostics;
        ComPtr<slang::IModule> module;
        module = session->loadModule("future", diagnostics.writeRef());
        SLANG_CHECK_ABORT(module != nullptr);
        SLANG_CHECK(moduleHasDirectDeclaration(module, toSlice("sourceOnly")));
        SLANG_CHECK(!moduleHasDirectDeclaration(module, toSlice("binaryOnly")));

        auto text = getDiagnosticText(diagnostics);
        SLANG_CHECK(countOccurrences(text, toSlice("warning[E00120]")) == 1);
    }
}
