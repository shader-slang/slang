#include "slang-global-session.h"
#include "slang-session.h"
#include "slang-filesystem.h"
#include "../../slang/slang-compiler.h"
#include "../util/capture-utility.h"

namespace SlangCapture
{
    // constructor is called in slang_createGlobalSession
    GlobalSessionCapture::GlobalSessionCapture(slang::IGlobalSession* session):
        m_actualGlobalSession(session)
    {
        SLANG_CAPTURE_ASSERT(m_actualGlobalSession != nullptr);

        m_globalSessionHandle = reinterpret_cast<SlangCapture::AddressFormat>(m_actualGlobalSession.get());
        m_captureManager = std::make_unique<CaptureManager>(m_globalSessionHandle);

        // We will use the address of the global session as the filename for the capture manager
        // to make it unique for each global session.
        // capture slang::createGlobalSession
        ParameterEncoder* encoder = m_captureManager->beginMethodCapture(ApiCallId::CreateGlobalSession, g_globalFunctionHandle);
        encoder->encodeAddress(m_actualGlobalSession);
        m_captureManager->endMethodCapture();
    }

    GlobalSessionCapture::~GlobalSessionCapture()
    {
        m_actualGlobalSession->release();
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::queryInterface(SlangUUID const& uuid, void** outObject) 
    {
        if (uuid == Session::getTypeGuid())
        {
            // no add-ref here, the query will cause the inner session to handle the add-ref.
            this->m_actualGlobalSession->queryInterface(uuid, outObject);
            return SLANG_OK;
        }

        if (uuid == ISlangUnknown::getTypeGuid() && uuid == IGlobalSession::getTypeGuid())
        {
            addReference();
            *outObject = static_cast<slang::IGlobalSession*>(this);
            return SLANG_OK;
        }

        return SLANG_E_NO_INTERFACE;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::createSession(slang::SessionDesc const&  desc, slang::ISession** outSession)
    {
        setLogLevel();
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        slang::ISession* actualSession = nullptr;

        ParameterEncoder* encoder{};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_createSession, m_globalSessionHandle);
            encoder->encodeStruct(desc);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->createSession(desc, &actualSession);

        {   // capture output
            encoder->encodeAddress(actualSession);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (actualSession != nullptr)
        {
            // reset the file system to our capture file system. After createSession() call,
            // the Linkage will set to user provided file system or slang default file system.
            // We need to reset it to our capture file system
            Slang::Linkage* linkage = static_cast<Linkage*>(actualSession);
            FileSystemCapture* fileSystemCapture = new FileSystemCapture(linkage->getFileSystemExt(), m_captureManager.get());

            Slang::ComPtr<FileSystemCapture> resultFileSystemCapture(fileSystemCapture);
            linkage->setFileSystem(resultFileSystemCapture.detach());

            SessionCapture* sessionCapture = new SessionCapture(actualSession, m_captureManager.get());
            Slang::ComPtr<SessionCapture> result(sessionCapture);
            *outSession = result.detach();
        }

        return res;
    }

    SLANG_NO_THROW SlangProfileID SLANG_MCALL GlobalSessionCapture::findProfile(char const* name)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_findProfile, m_globalSessionHandle);
            encoder->encodeString(name);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangProfileID profileId = m_actualGlobalSession->findProfile(name);
        return profileId;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setDownstreamCompilerPath(SlangPassThrough passThrough, char const* path)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_setDownstreamCompilerPath, m_globalSessionHandle);
            encoder->encodeEnumValue(passThrough);
            encoder->encodeString(path);
            m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->setDownstreamCompilerPath(passThrough, path);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setDownstreamCompilerPrelude(SlangPassThrough inPassThrough, char const* prelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_setDownstreamCompilerPrelude, m_globalSessionHandle);
            encoder->encodeEnumValue(inPassThrough);
            encoder->encodeString(prelude);
            m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->setDownstreamCompilerPrelude(inPassThrough, prelude);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::getDownstreamCompilerPrelude(SlangPassThrough inPassThrough, ISlangBlob** outPrelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_getDownstreamCompilerPrelude, m_globalSessionHandle);
            encoder->encodeEnumValue(inPassThrough);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->getDownstreamCompilerPrelude(inPassThrough, outPrelude);

        {
            encoder->encodeAddress(*outPrelude);
            m_captureManager->endMethodCaptureAppendOutput();
        }
    }

    SLANG_NO_THROW const char* SLANG_MCALL GlobalSessionCapture::getBuildTagString()
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        // No need to capture this function. It's just a query function and it won't impact the internal state.
        const char* resStr = m_actualGlobalSession->getBuildTagString();
        return resStr;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::setDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_setDefaultDownstreamCompiler, m_globalSessionHandle);
            encoder->encodeEnumValue(sourceLanguage);
            encoder->encodeEnumValue(defaultCompiler);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->setDefaultDownstreamCompiler(sourceLanguage, defaultCompiler);
        return res;
    }

    SLANG_NO_THROW SlangPassThrough SLANG_MCALL GlobalSessionCapture::getDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_getDefaultDownstreamCompiler, m_globalSessionHandle);
            encoder->encodeEnumValue(sourceLanguage);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangPassThrough passThrough = m_actualGlobalSession->getDefaultDownstreamCompiler(sourceLanguage);
        return passThrough;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setLanguagePrelude(SlangSourceLanguage inSourceLanguage, char const* prelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_setLanguagePrelude, m_globalSessionHandle);
            encoder->encodeEnumValue(inSourceLanguage);
            encoder->encodeString(prelude);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->setLanguagePrelude(inSourceLanguage, prelude);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::getLanguagePrelude(SlangSourceLanguage inSourceLanguage, ISlangBlob** outPrelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_getLanguagePrelude, m_globalSessionHandle);
            encoder->encodeEnumValue(inSourceLanguage);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->getLanguagePrelude(inSourceLanguage, outPrelude);

        {
            encoder->encodeAddress(*outPrelude);
            m_captureManager->endMethodCaptureAppendOutput();
        }
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::createCompileRequest(slang::ICompileRequest** outCompileRequest)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_createCompileRequest, m_globalSessionHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->createCompileRequest(outCompileRequest);

        {
            encoder->encodeAddress(*outCompileRequest);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::addBuiltins(char const* sourcePath, char const* sourceString)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_addBuiltins, m_globalSessionHandle);
            encoder->encodeString(sourcePath);
            encoder->encodeString(sourceString);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->addBuiltins(sourcePath, sourceString);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setSharedLibraryLoader(ISlangSharedLibraryLoader* loader)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        // TODO: Not sure if we need to capture this function. Because this functions is something like the file system
        // override, it's provided by user code. So capturing it makes no sense. The only way is to wrapper this interface
        // by our own implementation, and capture it there.
        m_actualGlobalSession->setSharedLibraryLoader(loader);
    }

    SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL GlobalSessionCapture::getSharedLibraryLoader()
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_getSharedLibraryLoader, m_globalSessionHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        ISlangSharedLibraryLoader* loader = m_actualGlobalSession->getSharedLibraryLoader();

        {
            encoder->encodeAddress(loader);
            m_captureManager->endMethodCaptureAppendOutput();
        }
        return loader;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::checkCompileTargetSupport(SlangCompileTarget target)
    {
        // No need to capture this function. It's just a query function and it won't impact the internal state.
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        SlangResult res = m_actualGlobalSession->checkCompileTargetSupport(target);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::checkPassThroughSupport(SlangPassThrough passThrough)
    {
        // No need to capture this function. It's just a query function and it won't impact the internal state.
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        SlangResult res = m_actualGlobalSession->checkPassThroughSupport(passThrough);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::compileStdLib(slang::CompileStdLibFlags flags)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_compileStdLib, m_globalSessionHandle);
            encoder->encodeEnumValue(flags);
            m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->compileStdLib(flags);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::loadStdLib(const void* stdLib, size_t stdLibSizeInBytes)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_loadStdLib, m_globalSessionHandle);
            encoder->encodePointer(stdLib, false, stdLibSizeInBytes);
            m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->loadStdLib(stdLib, stdLibSizeInBytes);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::saveStdLib(SlangArchiveType archiveType, ISlangBlob** outBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_saveStdLib, m_globalSessionHandle);
            encoder->encodeEnumValue(archiveType);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->saveStdLib(archiveType, outBlob);

        {
            encoder->encodeAddress(*outBlob);
            m_captureManager->endMethodCaptureAppendOutput();
        }
        return res;
    }

    SLANG_NO_THROW SlangCapabilityID SLANG_MCALL GlobalSessionCapture::findCapability(char const* name)
    {
        // No need to capture this function. It's just a query function and it won't impact the internal state.
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        SlangCapabilityID capId = m_actualGlobalSession->findCapability(name);
        return capId;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_setDownstreamCompilerForTransition, m_globalSessionHandle);
            encoder->encodeEnumValue(source);
            encoder->encodeEnumValue(target);
            encoder->encodeEnumValue(compiler);
            m_captureManager->endMethodCapture();
        }

        m_actualGlobalSession->setDownstreamCompilerForTransition(source, target, compiler);
    }

    SLANG_NO_THROW SlangPassThrough SLANG_MCALL GlobalSessionCapture::getDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target)
    {
        // No need to capture this function. It's just a query function and it won't impact the internal state.
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        SlangPassThrough passThrough = m_actualGlobalSession->getDownstreamCompilerForTransition(source, target);
        return passThrough;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime)
    {
        // No need to capture this function. It's just a query function and it won't impact the internal state.
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);
        m_actualGlobalSession->getCompilerElapsedTime(outTotalTime, outDownstreamTime);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::setSPIRVCoreGrammar(char const* jsonPath)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_setSPIRVCoreGrammar, m_globalSessionHandle);
            encoder->encodeString(jsonPath);
            m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->setSPIRVCoreGrammar(jsonPath);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::parseCommandLineArguments(
        int argc, const char* const* argv, slang::SessionDesc* outSessionDesc, ISlangUnknown** outAllocation)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_parseCommandLineArguments, m_globalSessionHandle);
            encoder->encodeInt32(argc);
            encoder->encodeStringArray(argv, argc);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->parseCommandLineArguments(argc, argv, outSessionDesc, outAllocation);

        {
            encoder->encodeAddress(outSessionDesc);
            encoder->encodeAddress(*outAllocation);
            m_captureManager->endMethodCaptureAppendOutput();
        }
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IGlobalSession_getSessionDescDigest, m_globalSessionHandle);
            encoder->encodeStruct(*sessionDesc);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualGlobalSession->getSessionDescDigest(sessionDesc, outBlob);

        {
            encoder->encodeAddress(*outBlob);
            m_captureManager->endMethodCaptureAppendOutput();
        }
        return res;
    }
}
