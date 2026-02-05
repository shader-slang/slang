#ifndef SLANG_PROXY_GLOBAL_SESSION_H
#define SLANG_PROXY_GLOBAL_SESSION_H

#include "proxy-base.h"
#include "proxy-macros.h"
#include "proxy-mutable-file-system.h"

#include "../../core/slang-file-system.h"
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

    // IGlobalSession
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override
    {
        RECORD_CALL();

        // Make a copy of the descriptor so we can modify the file system
        slang::SessionDesc modifiedDesc = desc;

        // Get the file system to wrap - use provided one or Slang's default
        //ISlangFileSystem* fileSystemToWrap =
        //    desc.fileSystem ? desc.fileSystem : OSFileSystem::getMutableSingleton();
        //modifiedDesc.fileSystem = static_cast<ISlangFileSystem*>(wrapObject(fileSystemToWrap));

        // Record the original descriptor (before our modification)
        RECORD_INPUT(desc);

        // Call create session with our wrapped file system
        slang::ISession* sessionPtr;
        if (!outSession)
            outSession = &sessionPtr;
        auto result = getActual<slang::IGlobalSession>()->createSession(modifiedDesc, outSession);

        RECORD_COM_OUTPUT(outSession);
        RECORD_RETURN(result);
    }

    /*
    // We could wrap up createSession in an even simpler way as it fits a very standard pattern
    // of taking a set of arguments and returning a single output. It would basically just expand
    // to the above version.
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override
    {
        // Single macro that performs the full logic shown above.
        RECORD_CALL_OUTPUT_OBJECT(slang::IGlobalSession, createSession, desc, outSession);
    }
    */

    virtual SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const* name) override
    {
        RECORD_CALL();
        RECORD_INPUT(name);
        auto result = getActual<slang::IGlobalSession>()->findProfile(name);
        RECORD_RETURN(result);
    }

    /*
    // Another standard pattern is a function that just takes some arguments and returns
    // a value. This could also be wrapped in a single macro.
    virtual SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const* name) override
    {
        RECORD_CALL_RETURN(slang::IGlobalSession, findProfile, name);
    }
    */


    virtual SLANG_NO_THROW void SLANG_MCALL
    setDownstreamCompilerPath(SlangPassThrough passThrough, char const* path) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNUSED(path);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setDownstreamCompilerPath");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setDownstreamCompilerPrelude(SlangPassThrough passThrough, const char* preludeText) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNUSED(preludeText);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setDownstreamCompilerPrelude");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getDownstreamCompilerPrelude(SlangPassThrough passThrough, ISlangBlob** outPrelude) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNUSED(outPrelude);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getDownstreamCompilerPrelude");
    }

    virtual SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() override
    {
        // No recording - this is just a version string query
        return getActual<slang::IGlobalSession>()->getBuildTagString();
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL setDefaultDownstreamCompiler(
        SlangSourceLanguage sourceLanguage,
        SlangPassThrough defaultCompiler) override
    {
        SLANG_UNUSED(sourceLanguage);
        SLANG_UNUSED(defaultCompiler);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setDefaultDownstreamCompiler");
    }

    virtual SlangPassThrough SLANG_MCALL
    getDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage) override
    {
        SLANG_UNUSED(sourceLanguage);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getDefaultDownstreamCompiler");
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
        if(!outPrelude)
            outPrelude = &preludePtr;
        getActual<IGlobalSession>()->getLanguagePrelude(sourceLanguage, outPrelude);
        RECORD_COM_OUTPUT(outPrelude);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompileRequest(slang::ICompileRequest** outCompileRequest) override
    {
        SLANG_UNUSED(outCompileRequest);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::createCompileRequest");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    addBuiltins(char const* sourcePath, char const* sourceString) override
    {
        SLANG_UNUSED(sourcePath);
        SLANG_UNUSED(sourceString);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::addBuiltins");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    setSharedLibraryLoader(ISlangSharedLibraryLoader* loader) override
    {
        SLANG_UNUSED(loader);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setSharedLibraryLoader");
    }

    virtual SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL getSharedLibraryLoader() override
    {
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getSharedLibraryLoader");
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
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::compileCoreModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadCoreModule(const void* coreModule, size_t coreModuleSizeInBytes) override
    {
        SLANG_UNUSED(coreModule);
        SLANG_UNUSED(coreModuleSizeInBytes);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::loadCoreModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    saveCoreModule(SlangArchiveType archiveType, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(archiveType);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::saveCoreModule");
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
        SLANG_UNUSED(source);
        SLANG_UNUSED(target);
        SLANG_UNUSED(compiler);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setDownstreamCompilerForTransition");
    }

    virtual SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDownstreamCompilerForTransition(
        SlangCompileTarget source,
        SlangCompileTarget target) override
    {
        RECORD_CALL();
        RECORD_INPUT(source);
        RECORD_INPUT(target);
        auto result = getActual<slang::IGlobalSession>()->getDownstreamCompilerForTransition(source, target);
        RECORD_RETURN(result);
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime) override
    {
        RECORD_CALL();
        
        double totalTime = 0.0;
        double downstreamTime = 0.0;
        if (!outTotalTime)
            outTotalTime = &totalTime;
        if (!outDownstreamTime)
            outDownstreamTime = &downstreamTime;

        getActual<slang::IGlobalSession>()->getCompilerElapsedTime(outTotalTime, outDownstreamTime);

        // These cause indeterminism
        // RECORD_INPUT(*outTotalTime);
        // RECORD_INPUT(*outDownstreamTime);
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setSPIRVCoreGrammar(char const* jsonPath) override
    {
        SLANG_UNUSED(jsonPath);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setSPIRVCoreGrammar");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL parseCommandLineArguments(
        int argc,
        const char* const* argv,
        slang::SessionDesc* outSessionDesc,
        ISlangUnknown** outAuxAllocation) override
    {
        SLANG_UNUSED(argc);
        SLANG_UNUSED(argv);
        SLANG_UNUSED(outSessionDesc);
        SLANG_UNUSED(outAuxAllocation);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::parseCommandLineArguments");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(sessionDesc);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getSessionDescDigest");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compileBuiltinModule(
        slang::BuiltinModuleName module,
        slang::CompileCoreModuleFlags flags) override
    {
        SLANG_UNUSED(module);
        SLANG_UNUSED(flags);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::compileBuiltinModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadBuiltinModule(
        slang::BuiltinModuleName module,
        const void* moduleData,
        size_t sizeInBytes) override
    {
        SLANG_UNUSED(module);
        SLANG_UNUSED(moduleData);
        SLANG_UNUSED(sizeInBytes);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::loadBuiltinModule");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL saveBuiltinModule(
        slang::BuiltinModuleName module,
        SlangArchiveType archiveType,
        ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(module);
        SLANG_UNUSED(archiveType);
        SLANG_UNUSED(outBlob);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::saveBuiltinModule");
    }
};

} // namespace SlangRecord

#endif // SLANG_PROXY_GLOBAL_SESSION_H
