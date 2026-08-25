// unit-test-module-version-range.cpp
//
// Regression tests for shader-slang/slang#12758 (out-of-range serialized module
// version). The version is byte-patched rather than written by the compiler
// because this build always emits `k_maxSupportedModuleVersion`, so it cannot
// produce an out-of-range module for itself to load.

#include "core/slang-riff.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-fossil.h"
#include "slang/slang-ir.h"
#include "slang/slang-serialize-container.h"
#include "slang/slang-serialize-ir.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

// Field indices into the fossilized `IRModuleInfo` and `IRModule` records, as
// declared in `slang-serialize-ir.cpp` (`IRModuleInfo`: serializationVersion,
// fullVersion, module; `Fossilized_IRModule`: m_name, m_version, m_moduleInst).
// The kind assertions below catch a drift in these indices at runtime.
static const Index kModuleInfoModuleFieldIndex = 2;
static const Index kModuleVersionFieldIndex = 1;

// Locate the mutable address of the serialized `IRModule::m_version` within a
// module-container blob held in `containerData`, reusing the reader's own fossil
// navigation so no byte offset is hard-coded. Returns nullptr if the blob is not
// shaped as expected (which would itself be a test failure).
uint64_t* findSerializedModuleVersion(void* containerData, size_t containerSize)
{
    auto rootChunk = RIFF::RootChunk::getFromBlob(containerData, containerSize);
    if (!rootChunk)
        return nullptr;

    auto moduleChunk = ModuleChunk::find(rootChunk);
    if (!moduleChunk)
        return nullptr;

    auto irChunk = moduleChunk->findIR();
    if (!irChunk)
        return nullptr;

    auto dataChunk = as<RIFF::DataChunk>(irChunk);
    if (!dataChunk)
        return nullptr;

    // The IR chunk payload is a fossil blob whose root is the `IRModuleInfo`
    // record. `getPayload()` points into `containerData`, so the address we
    // ultimately return is writable.
    Fossil::AnyValPtr rootValPtr =
        Fossil::getRootValue(dataChunk->getPayload(), dataChunk->getPayloadSize());
    if (!rootValPtr || rootValPtr->getKind() != FossilizedValKind::Struct)
        return nullptr;

    auto moduleInfoRecord = Fossil::cast<FossilizedRecordVal>(rootValPtr);

    // The `module` field is a fossilized pointer; follow it to the `IRModule`
    // record.
    Fossil::AnyValRef moduleFieldRef = moduleInfoRecord->getField(kModuleInfoModuleFieldIndex);
    if (moduleFieldRef.getKind() != FossilizedValKind::Ptr)
        return nullptr;

    auto modulePtr = Fossil::cast<FossilizedPtr<void>>(Fossil::getAddress(moduleFieldRef));
    Fossil::AnyValPtr moduleValPtr = modulePtr->getTargetValPtr();
    if (!moduleValPtr || moduleValPtr->getKind() != FossilizedValKind::Struct)
        return nullptr;

    auto moduleRecord = Fossil::cast<FossilizedRecordVal>(moduleValPtr);
    Fossil::AnyValRef versionFieldRef = moduleRecord->getField(kModuleVersionFieldIndex);
    if (versionFieldRef.getKind() != FossilizedValKind::UInt64)
        return nullptr;

    return static_cast<uint64_t*>(versionFieldRef.getDataPtr());
}

RIFF::Chunk const* findIRChunk(void const* containerData, size_t containerSize)
{
    auto rootChunk = RIFF::RootChunk::getFromBlob(containerData, containerSize);
    if (!rootChunk)
        return nullptr;
    auto moduleChunk = ModuleChunk::find(rootChunk);
    if (!moduleChunk)
        return nullptr;
    return moduleChunk->findIR();
}

// The exact "(min to max)" fragment the `unsupported-module-version` diagnostic
// embeds, so assertions match both bounds together rather than searching for a
// bare single digit that could occur elsewhere in the message.
String supportedRangeFragment()
{
    StringBuilder sb;
    sb << "(" << IRModule::k_minSupportedModuleVersion << " to "
       << IRModule::k_maxSupportedModuleVersion << ")";
    return sb.produceString();
}

bool serializeTrivialModule(UnitTestContext* unitTestContext, List<uint8_t>& outContainer)
{
    ComPtr<slang::ISession> session;
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;

        if (SLANG_FAILED(unitTestContext->slangGlobalSession->createSession(
                sessionDesc,
                session.writeRef())))
            return false;
    }

    const char* moduleSource = R"(
        module version_range_test;
        public int addOne(int x) { return x + 1; }
    )";

    ComPtr<slang::IModule> module;
    {
        ComPtr<slang::IBlob> diagnostics;
        module = session->loadModuleFromSourceString(
            "version_range_test",
            "version_range_test.slang",
            moduleSource,
            diagnostics.writeRef());
    }
    if (!module)
        return false;

    ComPtr<ISlangBlob> serializedBlob;
    if (SLANG_FAILED(module->serialize(serializedBlob.writeRef())) || !serializedBlob)
        return false;

    const auto* bytes = static_cast<const uint8_t*>(serializedBlob->getBufferPointer());
    const size_t size = serializedBlob->getBufferSize();
    outContainer.clear();
    outContainer.addRange(bytes, size);
    return true;
}

} // namespace

SLANG_UNIT_TEST(moduleVersionAboveMaxIsRejected)
{
    List<uint8_t> container;
    SLANG_CHECK_ABORT(serializeTrivialModule(unitTestContext, container));

    uint64_t* versionPtr = findSerializedModuleVersion(container.getBuffer(), container.getCount());
    SLANG_CHECK_ABORT(versionPtr != nullptr);

    // A freshly serialized module records exactly `k_maxSupportedModuleVersion`.
    SLANG_CHECK(*versionPtr == IRModule::k_maxSupportedModuleVersion);

    *versionPtr = IRModule::k_maxSupportedModuleVersion + 1;

    ComPtr<slang::ISession> session;
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));
    }

    // Failing cleanly here (rather than crashing) is the property under test.
    {
        ComPtr<slang::IBlob> diagnostics;
        slang::IModule* loaded = slang_loadModuleFromIRBlob(
            session,
            "version_range_test_loaded",
            "version_range_test_loaded.slang",
            container.getBuffer(),
            container.getCount(),
            diagnostics.writeRef());
        SLANG_CHECK(loaded == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr && diagnostics->getBufferSize() > 0);
        UnownedStringSlice message(
            static_cast<const char*>(diagnostics->getBufferPointer()),
            diagnostics->getBufferSize());
        // The message must name the offending version and the supported range.
        SLANG_CHECK(message.indexOf(UnownedStringSlice("unsupported")) != -1);
        SLANG_CHECK(
            message.indexOf(String(IRModule::k_maxSupportedModuleVersion + 1).getUnownedSlice()) !=
            -1);
        SLANG_CHECK(message.indexOf(supportedRangeFragment().getUnownedSlice()) != -1);
    }

    // The lightweight info path must remain functional for an out-of-range
    // module and report the (patched) version, so incompatible modules can still
    // be inspected via `-get-module-info`.
    {
        SlangInt moduleVersion = 0;
        const char* compilerVersion = nullptr;
        const char* moduleName = nullptr;
        SlangResult result = slang_loadModuleInfoFromIRBlob(
            session,
            container.getBuffer(),
            container.getCount(),
            moduleVersion,
            compilerVersion,
            moduleName);
        SLANG_CHECK(result == SLANG_OK);
        SLANG_CHECK(moduleVersion == SlangInt(IRModule::k_maxSupportedModuleVersion + 1));
    }
}

SLANG_UNIT_TEST(moduleVersionBelowMinIsRejected)
{
    // Only meaningful if there is a representable version below the minimum.
    if (IRModule::k_minSupportedModuleVersion == 0)
        return;

    List<uint8_t> container;
    SLANG_CHECK_ABORT(serializeTrivialModule(unitTestContext, container));

    uint64_t* versionPtr = findSerializedModuleVersion(container.getBuffer(), container.getCount());
    SLANG_CHECK_ABORT(versionPtr != nullptr);

    *versionPtr = IRModule::k_minSupportedModuleVersion - 1;

    ComPtr<slang::ISession> session;
    {
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            unitTestContext->slangGlobalSession->createSession(sessionDesc, session.writeRef())));
    }

    const uint64_t patchedVersion = IRModule::k_minSupportedModuleVersion - 1;

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* loaded = slang_loadModuleFromIRBlob(
        session,
        "version_range_test_below_min",
        "version_range_test_below_min.slang",
        container.getBuffer(),
        container.getCount(),
        diagnostics.writeRef());
    SLANG_CHECK(loaded == nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr && diagnostics->getBufferSize() > 0);
    UnownedStringSlice message(
        static_cast<const char*>(diagnostics->getBufferPointer()),
        diagnostics->getBufferSize());
    SLANG_CHECK(message.indexOf(UnownedStringSlice("unsupported")) != -1);
    SLANG_CHECK(message.indexOf(String(patchedVersion).getUnownedSlice()) != -1);
    SLANG_CHECK(message.indexOf(supportedRangeFragment().getUnownedSlice()) != -1);

    SlangInt moduleVersion = 0;
    const char* compilerVersion = nullptr;
    const char* moduleName = nullptr;
    SLANG_CHECK(
        slang_loadModuleInfoFromIRBlob(
            session,
            container.getBuffer(),
            container.getCount(),
            moduleVersion,
            compilerVersion,
            moduleName) == SLANG_OK);
    SLANG_CHECK(moduleVersion == SlangInt(patchedVersion));
}

// The choke-point reader `readSerializedModuleIR` itself rejects an out-of-range
// version, independently of the diagnostic emitted by the sink-bearing caller.
// This is the direct test of the mandatory safety check: the public
// `slang_loadModuleFromIRBlob` path above returns before ever calling
// `readSerializedModuleIR` (it fails at the diagnostic layer first), so without
// this case the reader-level guard that also protects the sink-less load paths
// (the global-session loader and the round-trip verify path) would be untested.
SLANG_UNIT_TEST(readSerializedModuleIRRejectsOutOfRangeVersion)
{
    StaticUnitTestEnv env(unitTestContext);

    List<uint8_t> container;
    SLANG_CHECK_ABORT(serializeTrivialModule(unitTestContext, container));

    uint64_t* versionPtr = findSerializedModuleVersion(container.getBuffer(), container.getCount());
    SLANG_CHECK_ABORT(versionPtr != nullptr);

    // Baseline: the unpatched, in-range module deserializes successfully through
    // the same reader, so a later failure is attributable to the version and not
    // to a malformed blob.
    {
        RIFF::Chunk const* irChunk = findIRChunk(container.getBuffer(), container.getCount());
        SLANG_CHECK_ABORT(irChunk != nullptr);
        RefPtr<IRModule> irModule;
        SLANG_CHECK(SLANG_SUCCEEDED(
            readSerializedModuleIR(irChunk, env.getSessionImpl(), nullptr, irModule)));
        SLANG_CHECK(irModule != nullptr);
    }

    *versionPtr = IRModule::k_maxSupportedModuleVersion + 1;
    {
        RIFF::Chunk const* irChunk = findIRChunk(container.getBuffer(), container.getCount());
        SLANG_CHECK_ABORT(irChunk != nullptr);
        RefPtr<IRModule> irModule;
        SLANG_CHECK(
            SLANG_FAILED(readSerializedModuleIR(irChunk, env.getSessionImpl(), nullptr, irModule)));
    }

    // Guard the subtraction: only exercise the below-min case when a version
    // below the minimum is representable.
    if (IRModule::k_minSupportedModuleVersion != 0)
    {
        *versionPtr = IRModule::k_minSupportedModuleVersion - 1;
        RIFF::Chunk const* irChunk = findIRChunk(container.getBuffer(), container.getCount());
        SLANG_CHECK_ABORT(irChunk != nullptr);
        RefPtr<IRModule> irModule;
        SLANG_CHECK(
            SLANG_FAILED(readSerializedModuleIR(irChunk, env.getSessionImpl(), nullptr, irModule)));
    }
}
