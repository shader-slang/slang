#include "core/slang-archive-file-system.h"
#include "core/slang-blob.h"
#include "slang/slang-ast-builder.h"
#include "slang/slang-global-session.h"
#include "slang/slang-serialize-types.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// Report when the last reference to the module's storage goes away.
class TrackedModuleBlob : public BlobBase
{
public:
    TrackedModuleBlob(ISlangBlob* contents, bool* destroyed)
        : m_contents(contents), m_destroyed(destroyed)
    {
    }
    ~TrackedModuleBlob() { *m_destroyed = true; }
    SLANG_NO_THROW const void* SLANG_MCALL getBufferPointer() override
    {
        return m_contents->getBufferPointer();
    }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override
    {
        return m_contents->getBufferSize();
    }

private:
    ComPtr<ISlangBlob> m_contents;
    bool* m_destroyed;
};

// Obtain a raw module even when the build uses compressed embedded archives.
SlangResult getSerializedCoreModule(UnitTestContext* context, ComPtr<ISlangBlob>& contents)
{
    ComPtr<ISlangBlob> archive;
    SLANG_RETURN_ON_FAIL(
        context->slangGlobalSession->saveCoreModule(SLANG_ARCHIVE_TYPE_RIFF, archive.writeRef()));
    ComPtr<ISlangFileSystemExt> fileSystem;
    SLANG_RETURN_ON_FAIL(
        loadArchiveFileSystem(archive->getBufferPointer(), archive->getBufferSize(), fileSystem));
    return fileSystem->loadFile("core.slang-module", contents.writeRef());
}

// These overloads and generic intrinsics need lazy builtin AST lookups after the input owner
// has gone out of scope. Request code as well so deserialized IR is exercised too.
SlangResult compileWithBuiltins(slang::IGlobalSession* globalSession)
{
    slang::TargetDesc target = {};
    target.format = SLANG_SPIRV;
    slang::SessionDesc desc = {};
    desc.targets = &target;
    desc.targetCount = 1;
    ComPtr<slang::ISession> session;
    SLANG_RETURN_ON_FAIL(globalSession->createSession(desc, session.writeRef()));
    ComPtr<ISlangBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "builtinStorage",
        "builtinStorage.slang",
        "RWStructuredBuffer<float4> output;\n"
        "[numthreads(1, 1, 1)] void main(uint3 tid : SV_DispatchThreadID)\n"
        "{ output[tid.x] = sin(float4(tid, 1)) + mul(float4x4(1.0), float4(2.0)); }\n",
        diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;
    ComPtr<slang::IEntryPoint> entry;
    SLANG_RETURN_ON_FAIL(module->findEntryPointByName("main", entry.writeRef()));
    slang::IComponentType* components[] = {module, entry};
    ComPtr<slang::IComponentType> program;
    SLANG_RETURN_ON_FAIL(session->createCompositeComponentType(
        components,
        2,
        program.writeRef(),
        diagnostics.writeRef()));
    ComPtr<ISlangBlob> code;
    SLANG_RETURN_ON_FAIL(program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef()));
    return code && code->getBufferSize() ? SLANG_OK : SLANG_FAIL;
}
} // namespace

SLANG_UNIT_TEST(builtinModuleBlobRetainsStorage)
{
    bool destroyed = false;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_createGlobalSessionWithoutCoreModule(
            SLANG_API_VERSION,
            globalSession.writeRef())));
        auto session = static_cast<Session*>(globalSession.get());
        {
            ComPtr<ISlangBlob> contents;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getSerializedCoreModule(unitTestContext, contents)));
            ComPtr<ISlangBlob> tracked(new TrackedModuleBlob(contents, &destroyed));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                session->loadBuiltinModule(slang::BuiltinModuleName::Core, tracked)));
            SLANG_CHECK(
                SLANG_FAILED(session->loadBuiltinModule(slang::BuiltinModuleName::Core, tracked)));
        }
        // A copy would release the original here. Borrowing without retaining it would also
        // release it, leaving the lazy AST with dangling pointers.
        SLANG_CHECK(!destroyed);
        SLANG_CHECK(SLANG_SUCCEEDED(compileWithBuiltins(globalSession)));
    }
    SLANG_CHECK(destroyed);
}

SLANG_UNIT_TEST(builtinModuleRawPointerOwnsStorage)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, globalSession.writeRef())));
    {
        ComPtr<ISlangBlob> contents;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(getSerializedCoreModule(unitTestContext, contents)));
        List<uint8_t> callerBytes;
        callerBytes.addRange(
            (const uint8_t*)contents->getBufferPointer(),
            contents->getBufferSize());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            globalSession->loadCoreModule(callerBytes.getBuffer(), callerBytes.getCount())));
        // The public raw-pointer API promises independence from the caller's input lifetime.
        memset(callerBytes.getBuffer(), 0, callerBytes.getCount());
    }
    SLANG_CHECK(SLANG_SUCCEEDED(compileWithBuiltins(globalSession)));
}

SLANG_UNIT_TEST(builtinModuleArchivesRemainSupported)
{
    for (auto type :
         {SLANG_ARCHIVE_TYPE_RIFF,
          SLANG_ARCHIVE_TYPE_RIFF_LZ4,
          SLANG_ARCHIVE_TYPE_RIFF_DEFLATE,
          SLANG_ARCHIVE_TYPE_ZIP})
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(slang_createGlobalSessionWithoutCoreModule(
            SLANG_API_VERSION,
            globalSession.writeRef())));
        {
            ComPtr<ISlangBlob> archive;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                unitTestContext->slangGlobalSession->saveCoreModule(type, archive.writeRef())));
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->loadCoreModule(
                archive->getBufferPointer(),
                archive->getBufferSize())));
        }
        SLANG_CHECK(SLANG_SUCCEEDED(compileWithBuiltins(globalSession)));
    }
}

SLANG_UNIT_TEST(builtinModuleEmbeddedStorageIsAligned)
{
#if SLANG_TEST_UNCOMPRESSED_BUILTINS
    auto embedded = slang_getEmbeddedCoreModule();
    if (!embedded)
    {
        SLANG_IGNORE_TEST;
        return;
    }
    SLANG_CHECK(size_t(embedded->getBufferPointer()) % RIFF::Chunk::kChunkAlignment == 0);
    auto root = RIFF::RootChunk::getFromBlob(embedded);
    SLANG_CHECK_ABORT(root);
    SLANG_CHECK(root->getType() == SerialBinary::kModuleFourCC);
#else
    SLANG_IGNORE_TEST;
#endif
}
