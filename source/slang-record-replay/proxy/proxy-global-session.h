#ifndef SLANG_PROXY_GLOBAL_SESSION_H
#define SLANG_PROXY_GLOBAL_SESSION_H

#include "core/slang-file-system.h"
#include "proxy-base.h"
#include "proxy-macros.h"
#include "proxy-mutable-file-system.h"
#include "slang-com-helper.h"
#include "slang.h"

#include <atomic>

namespace SlangRecord
{
using namespace Slang;

/// Live-instance count of `ReplayNullFileSystem`, for the unit test's deterministic
/// #12865 leak guard. Exported (`SLANG_API`, default visibility) on purpose: the
/// build uses `-fvisibility=hidden`, and the stand-in is constructed/destroyed inside
/// libslang (from `createSession`'s playback arm) while the test observes the count
/// from the separately-linked `slang-unit-test-tool` module. A plain (hidden) class
/// static would resolve to a *different* copy in each module and always read zero
/// from the test; a single exported symbol makes both modules share one counter (the
/// same reason `wrapObject`/`ReplayContext::get` are `SLANG_API`).
SLANG_API std::atomic<int>& testsOnlyReplayNullFileSystemLiveCount();

/// A properly reference-counted no-op file system used as the replay stand-in
/// for a recorded custom file system on the reading `kCustomFileSystemHandle`
/// arm of `createSession` (see below).
///
/// The user's real custom file system is not available during playback, so a
/// placeholder is wrapped in its place. Behaviour matches `NULLFileSystem`
/// (all file I/O operations return `SLANG_E_NOT_AVAILABLE`), but the lifetime
/// model differs: `NULLFileSystem` is a singleton whose `addRef`/`release` are no-ops,
/// so a per-call heap instance of it could never reach a zero refcount and would
/// leak. This subclass counts references per instance and deletes itself on the
/// final release, so a fresh instance can be created for each replayed custom-FS
/// session -- giving each a distinct proxy registration, which preserves the
/// record/playback handle sequence -- without leaking.
///
/// Only `addRef`/`release` are overridden; `queryInterface` (from the
/// `SLANG_IUNKNOWN_QUERY_INTERFACE` macro) and `castAs` are inherited from
/// `NULLFileSystem` unchanged. That is deliberate and load-bearing: the inherited
/// `queryInterface` calls `addRef()` *virtually*, so the reference the wrapper takes
/// while wrapping lands on this override's `m_refCount` rather than the base's no-op
/// counter. Inheriting the rest is therefore safe -- the only behaviour that must
/// differ from the singleton base is the reference counting, and it does.
class ReplayNullFileSystem : public NULLFileSystem
{
public:
    ReplayNullFileSystem() { ++testsOnlyReplayNullFileSystemLiveCount(); }
    ~ReplayNullFileSystem() SLANG_OVERRIDE { --testsOnlyReplayNullFileSystemLiveCount(); }

    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return ++m_refCount; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        uint32_t remaining = --m_refCount;
        if (remaining == 0)
            delete this;
        return remaining;
    }

private:
    std::atomic<uint32_t> m_refCount = 1;
};

class GlobalSessionProxy : public ProxyBase<slang::IGlobalSession>
{
public:
    SLANG_COM_INTERFACE(
        0x91a03c8f,
        0x6d7e,
        0x9f40,
        {0xd1, 0xc2, 0xbd, 0x8e, 0x7f, 0x60, 0x51, 0xe2})

    explicit GlobalSessionProxy(slang::IGlobalSession* actual)
        : ProxyBase(actual)
    {
    }

    // Record addRef/release for lifetime tracking during replay
    PROXY_REFCOUNT_IMPL(GlobalSessionProxy)

    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject)
        SLANG_OVERRIDE
    {
        if (!outObject)
            return SLANG_E_INVALID_ARG;

        if (uuid == GlobalSessionProxy::getTypeGuid() ||
            uuid == slang::IGlobalSession::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<slang::IGlobalSession*>(this);
            return SLANG_OK;
        }
        if (uuid == ISlangUnknown::getTypeGuid())
        {
            addRef();
            *outObject = static_cast<ISlangUnknown*>(static_cast<slang::IGlobalSession*>(this));
            return SLANG_OK;
        }
        // Unknown interface - pass through to underlying object
        return m_actual->queryInterface(uuid, outObject);
    }

    // IGlobalSession
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override
    {
        RECORD_CALL();

        // Record the original descriptor (before our modification)
        RECORD_INPUT(desc);

        // This logic handles the fact that a user can supply a custom file system, and we need to
        // make sure that on playback, we emulate EXACTLY the same behaviour. That means
        // distinguishing, between calling createSession twice with the same custom file system, or
        // twice with different ones. To do so, on writing we record a handle that identifies the
        // file system used, and on reading we use that to decide whether to look for an existing
        // proxy, wrap the OS, or wrap a new dummy 'NULL' filesystem.
        slang::SessionDesc desc2 = desc;
        // wrapObject returns an owning reference (see proxy-base.h). When we
        // create the wrapper here we are its caller, so we own that reference
        // and must release it once the real createSession has taken its own, or
        // the proxy leaks (issue #11936). The reading `default` branch instead
        // reuses an already-registered proxy via toSlangInterface, which is a
        // borrowed pointer, so it must not be released here.
        bool ownsFileSystemWrapper = false;
        if (_ctx.isWriting())
        {
            uint64_t handle = 0;
            if (desc.fileSystem)
            {
                if (_ctx.isInterfaceRegistered(desc.fileSystem))
                {
                    // Already wrapped once: wrapObject() returns the existing proxy
                    // and only adds a reference to that proxy, leaving the user's
                    // file system untouched, so no reference is owed on it here. That
                    // proxy reference is owning, and is balanced by the shared
                    // ownsFileSystemWrapper release() after the real createSession below.
                    desc2.fileSystem = wrapObject(desc.fileSystem);
                    handle = _ctx.getProxyHandle(desc2.fileSystem);
                }
                else
                {
                    // First wrap of this file system: wrapObject() -> tryWrap() takes
                    // ownership of one reference on it (it calls
                    // desc.fileSystem->release()), so pre-add one here for it to
                    // consume; otherwise it would consume the caller's own reference.
                    // (The already-registered branch above owes no such addRef -- see
                    // its comment.)
                    desc.fileSystem->addRef();
                    desc2.fileSystem = wrapObject(desc.fileSystem);
                    handle = kCustomFileSystemHandle;
                }
            }
            else
            {
                desc2.fileSystem = wrapObject(OSFileSystem::getMutableSingleton());
                handle = kDefaultFileSystemHandle;
            }
            ownsFileSystemWrapper = true;
            RECORD_INFO(handle);
        }
        else if (_ctx.isReading())
        {
            uint64_t handle;
            RECORD_INFO(handle);
            switch (handle)
            {
            case kDefaultFileSystemHandle:
                {
                    desc2.fileSystem = wrapObject(OSFileSystem::getMutableSingleton());
                    ownsFileSystemWrapper = true;
                    break;
                }
            case kCustomFileSystemHandle:
                {
                    // Create a fresh stand-in file system for the recorded custom
                    // file system (unavailable at playback). It must be a distinct
                    // object per call so each replayed custom-FS session takes its
                    // own proxy registration, matching the recorded handle sequence.
                    //
                    // Bind it through an ISlangMutableFileSystem* (not the concrete
                    // ReplayNullFileSystem*): the type-safe wrapObject<T> template QIs
                    // the wrapped proxy back to T, which is valid only when T is a COM
                    // interface with a getTypeGuid(). Deducing T as the concrete impl
                    // type makes toSlangInterface<T> call release() on the proxy through
                    // a wrong-typed pointer (undefined behaviour). Wrapping an interface
                    // pointer mirrors the default arm's
                    // wrapObject(OSFileSystem::getMutableSingleton()).
                    //
                    // wrapObject() adopts one reference (its inner tryWrap() calls
                    // release()), so hand over the single reference from `new` -- do not
                    // pre-add another, or the object would never be freed. The proxy's
                    // m_actual then holds the only reference, and ReplayNullFileSystem's
                    // per-instance ref counting deletes it once that proxy is destroyed.
                    ISlangMutableFileSystem* standIn = new ReplayNullFileSystem();
                    desc2.fileSystem = wrapObject(standIn);
                    ownsFileSystemWrapper = true;
                    break;
                }
            default:
                {
                    desc2.fileSystem = toSlangInterface<ISlangFileSystem>(_ctx.getProxy(handle));
                    break;
                }
            }
        }

        // Call create session with our wrapped file system
        PREPARE_POINTER_OUTPUT(outSession);
        auto result = getActual<slang::IGlobalSession>()->createSession(desc2, outSession);

        // The created session holds its own reference to the file system for its
        // lifetime, so drop the creation reference we still hold on the wrapper.
        // Suppress recording: this is our internal bookkeeping, not a user
        // release that belongs in the stream (issue #11936).
        if (ownsFileSystemWrapper && desc2.fileSystem)
        {
            SuppressRefCountRecording guard;
            desc2.fileSystem->release();
        }

        RECORD_COM_OUTPUT(outSession);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const* name) override
    {
        RECORD_CALL();
        RECORD_INPUT(name);
        auto result = getActual<slang::IGlobalSession>()->findProfile(name);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDownstreamCompilerPath(SlangPassThrough passThrough, char const* path) override
    {
        RECORD_CALL();
        RECORD_INPUT(passThrough);
        RECORD_INPUT(path);
        getActual<slang::IGlobalSession>()->setDownstreamCompilerPath(passThrough, path);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDownstreamCompilerPrelude(SlangPassThrough passThrough, const char* preludeText) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNUSED(preludeText);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::setDownstreamCompilerPrelude");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getDownstreamCompilerPrelude(SlangPassThrough passThrough, ISlangBlob** outPrelude) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNUSED(outPrelude);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::getDownstreamCompilerPrelude");
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() override
    {
        RECORD_CALL();
        auto result = getActual<slang::IGlobalSession>()->getBuildTagString();
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL setDefaultDownstreamCompiler(
        SlangSourceLanguage sourceLanguage,
        SlangPassThrough defaultCompiler) override
    {
        RECORD_CALL();
        RECORD_INPUT(sourceLanguage);
        RECORD_INPUT(defaultCompiler);
        auto result = getActual<IGlobalSession>()->setDefaultDownstreamCompiler(
            sourceLanguage,
            defaultCompiler);
        RECORD_RETURN(result);
    }

    virtual SlangPassThrough SLANG_MCALL
    getDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage) override
    {
        RECORD_CALL();
        RECORD_INPUT(sourceLanguage);
        auto result = getActual<IGlobalSession>()->getDefaultDownstreamCompiler(sourceLanguage);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setLanguagePrelude(SlangSourceLanguage sourceLanguage, const char* preludeText) override
    {
        RECORD_CALL();
        RECORD_INPUT(sourceLanguage);
        RECORD_INPUT(preludeText);
        getActual<IGlobalSession>()->setLanguagePrelude(sourceLanguage, preludeText);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getLanguagePrelude(SlangSourceLanguage sourceLanguage, ISlangBlob** outPrelude) override
    {
        RECORD_CALL();
        RECORD_INPUT(sourceLanguage);
        PREPARE_POINTER_OUTPUT(outPrelude);
        getActual<IGlobalSession>()->getLanguagePrelude(sourceLanguage, outPrelude);
        RECORD_COM_OUTPUT(outPrelude);
    }

    SLANG_ALLOW_DEPRECATED_BEGIN
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompileRequest(slang::ICompileRequest** outCompileRequest) override
    {
        RECORD_CALL();
        PREPARE_POINTER_OUTPUT(outCompileRequest);
        auto result = getActual<slang::IGlobalSession>()->createCompileRequest(outCompileRequest);
        RECORD_COM_OUTPUT(outCompileRequest);
        RECORD_RETURN(result);
    }
    SLANG_ALLOW_DEPRECATED_END

    virtual SLANG_NO_THROW void SLANG_MCALL
    addBuiltins(char const* sourcePath, char const* sourceString) override
    {
        SLANG_UNUSED(sourcePath);
        SLANG_UNUSED(sourceString);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::addBuiltins");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setSharedLibraryLoader(ISlangSharedLibraryLoader* loader) override
    {
        SLANG_UNUSED(loader);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::setSharedLibraryLoader");
    }

    virtual SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL getSharedLibraryLoader() override
    {
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::getSharedLibraryLoader");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    checkCompileTargetSupport(SlangCompileTarget target) override
    {
        RECORD_CALL();
        RECORD_INPUT(target);
        auto result = getActual<slang::IGlobalSession>()->checkCompileTargetSupport(target);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    checkPassThroughSupport(SlangPassThrough passThrough) override
    {
        RECORD_CALL();
        RECORD_INPUT(passThrough);
        auto result = getActual<slang::IGlobalSession>()->checkPassThroughSupport(passThrough);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getDownstreamCompilerPath(SlangPassThrough passThrough, ISlangBlob** outPath) override
    {
        RECORD_CALL();
        RECORD_INPUT(passThrough);
        PREPARE_POINTER_OUTPUT(outPath);
        auto result =
            getActual<slang::IGlobalSession>()->getDownstreamCompilerPath(passThrough, outPath);
        // On failure the actual API returns without writing *outPath. The record stream has a fixed
        // schema and must still serialize the output slot, so redirect to the zero-initialized
        // temporary created by PREPARE_POINTER_OUTPUT above and record a defined null instead of
        // reading the caller's uninitialized memory (see issue #11865). The caller's memory is left
        // untouched.
        if (SLANG_FAILED(result))
            outPath = &_temp_outPath;
        RECORD_COM_OUTPUT(outPath);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    compileCoreModule(slang::CompileCoreModuleFlags flags) override
    {
        SLANG_UNUSED(flags);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::compileCoreModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadCoreModule(const void* coreModule, size_t coreModuleSizeInBytes) override
    {
        SLANG_UNUSED(coreModule);
        SLANG_UNUSED(coreModuleSizeInBytes);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::loadCoreModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveCoreModule(SlangArchiveType archiveType, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(archiveType);
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::saveCoreModule");
    }

    virtual SLANG_NO_THROW SlangCapabilityID SLANG_MCALL findCapability(char const* name) override
    {
        RECORD_CALL();
        RECORD_INPUT(name);
        auto result = getActual<slang::IGlobalSession>()->findCapability(name);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerForTransition(
        SlangCompileTarget source,
        SlangCompileTarget target,
        SlangPassThrough compiler) override
    {
        RECORD_CALL();
        RECORD_INPUT(source);
        RECORD_INPUT(target);
        RECORD_INPUT(compiler);
        getActual<IGlobalSession>()->setDownstreamCompilerForTransition(source, target, compiler);
        RECORD_RETURN_VOID();
    }

    virtual SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDownstreamCompilerForTransition(
        SlangCompileTarget source,
        SlangCompileTarget target) override
    {
        RECORD_CALL();
        RECORD_INPUT(source);
        RECORD_INPUT(target);
        auto result =
            getActual<slang::IGlobalSession>()->getDownstreamCompilerForTransition(source, target);
        RECORD_RETURN(result);
    }

    // Note: Records the call, but not results, as they are not deterministic.
    virtual SLANG_NO_THROW void SLANG_MCALL
    getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime) override
    {
        RECORD_CALL();
        PREPARE_POINTER_OUTPUT(outTotalTime);
        PREPARE_POINTER_OUTPUT(outDownstreamTime);
        getActual<slang::IGlobalSession>()->getCompilerElapsedTime(outTotalTime, outDownstreamTime);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setSPIRVCoreGrammar(char const* jsonPath) override
    {
        SLANG_UNUSED(jsonPath);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::setSPIRVCoreGrammar");
    }

    // parseCommandLineArguments is a bit special because tracking the 'aux allocation' is
    // painful, and it only really acts as a holder for memory allocated for the session desc.
    // Instead, we store/return the session desc directly in the stream, and we ignore
    // the aux allocation entirely.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL parseCommandLineArguments(
        int argc,
        const char* const* argv,
        slang::SessionDesc* outSessionDesc,
        ISlangUnknown** outAuxAllocation) override
    {
        RECORD_CALL();
        RECORD_INPUT(argc);
        RECORD_INPUT_ARRAY(argv, argc);
        PREPARE_POINTER_OUTPUT(outSessionDesc);
        SlangResult result = SLANG_OK;
        if (ReplayContext::get().isWriting())
        {
            result = getActual<slang::IGlobalSession>()
                         ->parseCommandLineArguments(argc, argv, outSessionDesc, outAuxAllocation);
        }
        RECORD_INFO(*outSessionDesc);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob) override
    {
        RECORD_CALL();
        PREPARE_POINTER_INPUT(sessionDesc);
        RECORD_INPUT(*sessionDesc);
        PREPARE_POINTER_OUTPUT(outBlob);
        auto result =
            getActual<slang::IGlobalSession>()->getSessionDescDigest(sessionDesc, outBlob);
        RECORD_COM_OUTPUT(outBlob);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compileBuiltinModule(
        slang::BuiltinModuleName module,
        slang::CompileCoreModuleFlags flags) override
    {
        SLANG_UNUSED(module);
        SLANG_UNUSED(flags);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::compileBuiltinModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBuiltinModule(
        slang::BuiltinModuleName module,
        const void* moduleData,
        size_t sizeInBytes) override
    {
        SLANG_UNUSED(module);
        SLANG_UNUSED(moduleData);
        SLANG_UNUSED(sizeInBytes);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::loadBuiltinModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveBuiltinModule(
        slang::BuiltinModuleName module,
        SlangArchiveType archiveType,
        ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(module);
        SLANG_UNUSED(archiveType);
        SLANG_UNUSED(outBlob);
        REPLAY_UNIMPLEMENTED_X("GlobalSessionProxy::saveBuiltinModule");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_GLOBAL_SESSION_H
