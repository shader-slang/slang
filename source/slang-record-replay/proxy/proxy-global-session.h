#ifndef SLANG_PROXY_GLOBAL_SESSION_H
#define SLANG_PROXY_GLOBAL_SESSION_H

#include "proxy-base.h"

#include "../../core/slang-smart-pointer.h"
#include "slang-com-helper.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class GlobalSessionProxy : public slang::IGlobalSession,
                           public RefObject,
                           public ProxyBase
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

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    // IGlobalSession
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    createSession(slang::SessionDesc const& desc, slang::ISession** outSession) override
    {
        SLANG_UNUSED(desc);
        SLANG_UNUSED(outSession);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::createSession");
    }

    virtual SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const* name) override
    {
        SLANG_UNUSED(name);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::findProfile");
    }

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
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getBuildTagString");
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
        SLANG_UNUSED(sourceLanguage);
        SLANG_UNUSED(preludeText);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::setLanguagePrelude");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getLanguagePrelude(SlangSourceLanguage sourceLanguage, ISlangBlob** outPrelude) override
    {
        SLANG_UNUSED(sourceLanguage);
        SLANG_UNUSED(outPrelude);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getLanguagePrelude");
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
        SLANG_UNUSED(target);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::checkCompileTargetSupport");
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    checkPassThroughSupport(SlangPassThrough passThrough) override
    {
        SLANG_UNUSED(passThrough);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::checkPassThroughSupport");
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
        SLANG_UNUSED(name);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::findCapability");
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
        SLANG_UNUSED(source);
        SLANG_UNUSED(target);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getDownstreamCompilerForTransition");
    }

    virtual SLANG_NO_THROW void SLANG_MCALL
    getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime) override
    {
        SLANG_UNUSED(outTotalTime);
        SLANG_UNUSED(outDownstreamTime);
        SLANG_UNIMPLEMENTED_X("GlobalSessionProxy::getCompilerElapsedTime");
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
