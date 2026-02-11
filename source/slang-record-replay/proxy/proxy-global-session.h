#ifndef SLANG_PROXY_GLOBAL_SESSION_H
#define SLANG_PROXY_GLOBAL_SESSION_H

#include "../../core/slang-file-system.h"
#include "proxy-base.h"
#include "proxy-macros.h"
#include "proxy-mutable-file-system.h"
#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

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
        if (_ctx.isWriting())
        {
            uint64_t handle = 0;
            if (desc.fileSystem)
            {
                desc.fileSystem->addRef();
                if (_ctx.isInterfaceRegistered(desc.fileSystem))
                {
                    desc2.fileSystem = wrapObject(desc.fileSystem);
                    handle = _ctx.getProxyHandle(desc2.fileSystem);
                }
                else
                {
                    desc2.fileSystem = wrapObject(desc.fileSystem);
                    handle = kCustomFileSystemHandle;
                }
            }
            else
            {
                desc2.fileSystem = wrapObject(OSFileSystem::getMutableSingleton());
                handle = kDefaultFileSystemHandle;
            }
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
                    break;
                }
            case kCustomFileSystemHandle:
                {
                    auto nfs = new NULLFileSystem();
                    nfs->addRef();
                    desc2.fileSystem = wrapObject(nfs);
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
        ISlangBlob* preludePtr;
        if (!outPrelude)
            outPrelude = &preludePtr;
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
        PREPARE_POINTER_OUTPUT(sessionDesc);
        PREPARE_POINTER_OUTPUT(outBlob);
        RECORD_INPUT(*sessionDesc);
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
