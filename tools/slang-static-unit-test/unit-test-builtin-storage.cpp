#include "core/slang-archive-file-system.h"
#include "core/slang-blob.h"
#include "slang/slang-ast-builder.h"
#include "slang/slang-global-session.h"
#include "slang/slang-serialize-types.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <thread>

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

// Extract serialized core bytes for internal ownership tests.
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
        "[shader(\"compute\")][numthreads(1, 1, 1)] void main(uint3 tid : SV_DispatchThreadID)\n"
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
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->saveCoreModule(
            SLANG_ARCHIVE_TYPE_RIFF_LZ4,
            contents.writeRef())));
        List<uint8_t> callerBytes;
        // Public archives may begin at an unaligned address, for example inside a larger file.
        callerBytes.add(0);
        callerBytes.addRange(
            (const uint8_t*)contents->getBufferPointer(),
            contents->getBufferSize());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->loadCoreModule(
            callerBytes.getBuffer() + 1,
            callerBytes.getCount() - 1)));
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

SLANG_UNIT_TEST(builtinModuleArchiveSharesStorage)
{
    ComPtr<ISlangBlob> archiveBytes;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->saveCoreModule(
        SLANG_ARCHIVE_TYPE_RIFF_LZ4,
        archiveBytes.writeRef())));
    ComPtr<ISlangBlob> modules[8];
    {
        BuiltinModuleArchive archive(archiveBytes, slang::BuiltinModuleName::Core);
        SlangResult results[8] = {};
        std::thread threads[8];
        std::atomic<int> ready(0);
        std::atomic<bool> start(false);
        for (int i = 0; i < 8; ++i)
        {
            threads[i] = std::thread(
                [&, i]()
                {
                    ++ready;
                    while (!start.load())
                        std::this_thread::yield();
                    results[i] = archive.getModule(modules[i]);
                });
        }
        while (ready.load() != 8)
            std::this_thread::yield();
        start.store(true);
        for (auto& thread : threads)
            thread.join();
        for (int i = 0; i < 8; ++i)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(results[i]));
            SLANG_CHECK_ABORT(modules[i]);
            SLANG_CHECK(modules[i].get() == modules[0].get());
        }
        // The cache keeps storage across periods when none of its clients retain a reference.
        auto original = modules[0].get();
        for (auto& module : modules)
            module = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(archive.getModule(modules[0])));
        SLANG_CHECK(modules[0].get() == original);
    }
    archiveBytes = nullptr;
    // A returned reference remains usable even after the cache and input archive are destroyed.
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, globalSession.writeRef())));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(static_cast<Session*>(globalSession.get())
                            ->loadBuiltinModule(slang::BuiltinModuleName::Core, modules[0])));
    modules[0] = nullptr;
    SLANG_CHECK(SLANG_SUCCEEDED(compileWithBuiltins(globalSession)));
}

SLANG_UNIT_TEST(builtinModuleArchiveFailureDoesNotPublishStorage)
{
    // A valid archive with no GLSL entry must fail every time; core bytes must not be used as GLSL.
    ComPtr<ISlangBlob> archiveBytes;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(unitTestContext->slangGlobalSession->saveCoreModule(
        SLANG_ARCHIVE_TYPE_RIFF_LZ4,
        archiveBytes.writeRef())));
    BuiltinModuleArchive archive(archiveBytes, slang::BuiltinModuleName::GLSL);
    for (int i = 0; i < 2; ++i)
    {
        ComPtr<ISlangBlob> module = archiveBytes;
        SLANG_CHECK(SLANG_FAILED(archive.getModule(module)));
        SLANG_CHECK(!module);
    }
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, globalSession.writeRef())));
    SLANG_CHECK(SLANG_FAILED(globalSession->loadBuiltinModule(
        slang::BuiltinModuleName::GLSL,
        archiveBytes->getBufferPointer(),
        archiveBytes->getBufferSize())));
    // A failed load must leave the session usable for the correctly named module.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->loadCoreModule(
        archiveBytes->getBufferPointer(),
        archiveBytes->getBufferSize())));
    SLANG_CHECK(SLANG_SUCCEEDED(compileWithBuiltins(globalSession)));
}
