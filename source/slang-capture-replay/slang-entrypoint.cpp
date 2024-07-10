#include "capture_utility.h"
#include "slang-entrypoint.h"

namespace SlangCapture
{
    EntryPointCapture::EntryPointCapture(slang::IEntryPoint* entryPoint, CaptureManager* captureManager)
        : m_actualEntryPoint(entryPoint),
          m_captureManager(captureManager)
    {
        SLANG_CAPTURE_ASSERT(m_actualEntryPoint != nullptr);
        SLANG_CAPTURE_ASSERT(m_captureManager != nullptr);

        m_entryPointHandle = reinterpret_cast<uint64_t>(m_actualEntryPoint.get());
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, entryPoint);
    }

    EntryPointCapture::~EntryPointCapture()
    {
        m_actualEntryPoint->release();
    }

    ISlangUnknown* EntryPointCapture::getInterface(const Guid& guid)
    {
        if(guid == EntryPointCapture::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    SLANG_NO_THROW slang::ISession* EntryPointCapture::getSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getSession, m_entryPointHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ISession* session = m_actualEntryPoint->getSession();

        {
            encoder->encodeAddress(session);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return session;
    }

    SLANG_NO_THROW slang::ProgramLayout* EntryPointCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getLayout, m_entryPointHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ProgramLayout* programLayout = m_actualEntryPoint->getLayout(targetIndex, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(programLayout);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt EntryPointCapture::getSpecializationParamCount()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualEntryPoint->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getEntryPointCode, m_entryPointHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getTargetCode, m_entryPointHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getResultAsFileSystem, m_entryPointHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            encoder->encodeAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void EntryPointCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getEntryPointHash, m_entryPointHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualEntryPoint->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            encoder->encodeAddress(*outHash);
            m_captureManager->endMethodCaptureAppendOutput();
        }
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_specialize, m_entryPointHandle);
            encoder->encodeInt64(specializationArgCount);
            encoder->encodeStructArray(specializationArgs, specializationArgCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outSpecializedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_link, m_entryPointHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->link(outLinkedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_getEntryPointHostCallable, m_entryPointHandle);
            encoder->encodeInt32(entryPointIndex);
            encoder->encodeInt32(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            encoder->encodeAddress(*outSharedLibrary);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_renameEntryPoint, m_entryPointHandle);
            encoder->encodeString(newName);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->renameEntryPoint(newName, outEntryPoint);

        {
            encoder->encodeAddress(*outEntryPoint);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IEntryPoint_linkWithOptions, m_entryPointHandle);
            encoder->encodeUint32(compilerOptionEntryCount);
            encoder->encodeStructArray(compilerOptionEntries, compilerOptionEntryCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualEntryPoint->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW slang::FunctionReflection* EntryPointCapture::getFunctionReflection()
    {
        return m_actualEntryPoint->getFunctionReflection();
    }

}
