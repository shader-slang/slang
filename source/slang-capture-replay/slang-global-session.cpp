
#include <vector>
#include "slang-global-session.h"
#include "capture_utility.h"
#include "slang-session.h"
#include "slang-filesystem.h"
#include "../slang/slang-compiler.h"

namespace SlangCapture
{
    GlobalSessionCapture::GlobalSessionCapture(slang::IGlobalSession* session):
        m_actualGlobalSession(session)
    {
        assert(m_actualGlobalSession != nullptr);
    }

    GlobalSessionCapture::~GlobalSessionCapture()
    {
        m_actualGlobalSession->release();
    }

    ISlangUnknown* GlobalSessionCapture::getInterface(const Guid& guid)
    {
        if(guid == ISlangUnknown::getTypeGuid() || guid == IGlobalSession::getTypeGuid())
            return asExternal(this);
        return nullptr;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::createSession(slang::SessionDesc const&  desc, slang::ISession** outSession)
    {
        setLogLevel();
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);

        slang::ISession* actualSession = nullptr;
        SlangResult res = m_actualGlobalSession->createSession(desc, &actualSession);

        if (actualSession != nullptr)
        {
            // reset the file system to our capture file system. After createSession() call,
            // the Linkage will set to user provided file system or slang default file system.
            // We need to reset it to our capture file system
            Slang::Linkage* linkage = static_cast<Linkage*>(actualSession);
            FileSystemCapture* fileSystemCapture = new FileSystemCapture(linkage->getFileSystemExt());

            Slang::ComPtr<FileSystemCapture> resultFileSystemCapture(fileSystemCapture);
            linkage->setFileSystem(resultFileSystemCapture.detach());

            SessionCapture* sessionCapture = new SessionCapture(actualSession);
            Slang::ComPtr<SessionCapture> result(sessionCapture);
            *outSession = result.detach();
        }

        return res;
    }

    SLANG_NO_THROW SlangProfileID SLANG_MCALL GlobalSessionCapture::findProfile(char const* name)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangProfileID profileId = m_actualGlobalSession->findProfile(name);
        return profileId;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setDownstreamCompilerPath(SlangPassThrough passThrough, char const* path)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->setDownstreamCompilerPath(passThrough, path);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setDownstreamCompilerPrelude(SlangPassThrough inPassThrough, char const* prelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->setDownstreamCompilerPrelude(inPassThrough, prelude);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::getDownstreamCompilerPrelude(SlangPassThrough inPassThrough, ISlangBlob** outPrelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->getDownstreamCompilerPrelude(inPassThrough, outPrelude);
    }

    SLANG_NO_THROW const char* SLANG_MCALL GlobalSessionCapture::getBuildTagString()
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        const char* resStr = m_actualGlobalSession->getBuildTagString();
        return resStr;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::setDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->setDefaultDownstreamCompiler(sourceLanguage, defaultCompiler);
        return res;
    }

    SLANG_NO_THROW SlangPassThrough SLANG_MCALL GlobalSessionCapture::getDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangPassThrough passThrough = m_actualGlobalSession->getDefaultDownstreamCompiler(sourceLanguage);
        return passThrough;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setLanguagePrelude(SlangSourceLanguage inSourceLanguage, char const* prelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->setLanguagePrelude(inSourceLanguage, prelude);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::getLanguagePrelude(SlangSourceLanguage inSourceLanguage, ISlangBlob** outPrelude)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->getLanguagePrelude(inSourceLanguage, outPrelude);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::createCompileRequest(slang::ICompileRequest** outCompileRequest)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->createCompileRequest(outCompileRequest);
        return res;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::addBuiltins(char const* sourcePath, char const* sourceString)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->addBuiltins(sourcePath, sourceString);
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setSharedLibraryLoader(ISlangSharedLibraryLoader* loader)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->setSharedLibraryLoader(loader);
    }

    SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL GlobalSessionCapture::getSharedLibraryLoader()
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        ISlangSharedLibraryLoader* loader = m_actualGlobalSession->getSharedLibraryLoader();
        return loader;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::checkCompileTargetSupport(SlangCompileTarget target)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->checkCompileTargetSupport(target);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::checkPassThroughSupport(SlangPassThrough passThrough)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->checkPassThroughSupport(passThrough);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::compileStdLib(slang::CompileStdLibFlags flags)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->compileStdLib(flags);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::loadStdLib(const void* stdLib, size_t stdLibSizeInBytes)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->loadStdLib(stdLib, stdLibSizeInBytes);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::saveStdLib(SlangArchiveType archiveType, ISlangBlob** outBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->saveStdLib(archiveType, outBlob);
        return res;
    }

    SLANG_NO_THROW SlangCapabilityID SLANG_MCALL GlobalSessionCapture::findCapability(char const* name)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangCapabilityID capId = m_actualGlobalSession->findCapability(name);
        return capId;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::setDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->setDownstreamCompilerForTransition(source, target, compiler);
    }

    SLANG_NO_THROW SlangPassThrough SLANG_MCALL GlobalSessionCapture::getDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangPassThrough passThrough = m_actualGlobalSession->getDownstreamCompilerForTransition(source, target);
        return passThrough;
    }

    SLANG_NO_THROW void SLANG_MCALL GlobalSessionCapture::getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        m_actualGlobalSession->getCompilerElapsedTime(outTotalTime, outDownstreamTime);
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::setSPIRVCoreGrammar(char const* jsonPath)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->setSPIRVCoreGrammar(jsonPath);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::parseCommandLineArguments(
        int argc, const char* const* argv, slang::SessionDesc* outSessionDesc, ISlangUnknown** outAllocation)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->parseCommandLineArguments(argc, argv, outSessionDesc, outAllocation);
        return res;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL GlobalSessionCapture::getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%p: %s\n", m_actualGlobalSession.get(), __func__);
        SlangResult res = m_actualGlobalSession->getSessionDescDigest(sessionDesc, outBlob);
        return res;
    }
}
