#include "capture-utility.h"
#include "slang-type-conformance.h"

namespace SlangCapture
{
    TypeConformanceCapture::TypeConformanceCapture(slang::ITypeConformance* typeConformance, CaptureManager* captureManager)
        : m_actualTypeConformance(typeConformance),
          m_captureManager(captureManager)
    {
        SLANG_CAPTURE_ASSERT(m_actualTypeConformance != nullptr);
        SLANG_CAPTURE_ASSERT(m_captureManager != nullptr);

        m_typeConformanceHandle = reinterpret_cast<uint64_t>(m_actualTypeConformance.get());
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, typeConformance);
    }

    TypeConformanceCapture::~TypeConformanceCapture()
    {
        m_actualTypeConformance->release();
    }

    ISlangUnknown* TypeConformanceCapture::getInterface(const Guid& guid)
    {
        if (guid == TypeConformanceCapture::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        else
        {
            return nullptr;
        }
    }

    SLANG_NO_THROW slang::ISession* TypeConformanceCapture::getSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_getSession, m_typeConformanceHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ISession* res = m_actualTypeConformance->getSession();

        {
            encoder->encodeAddress(res);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* TypeConformanceCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getLayout, m_typeConformanceHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ProgramLayout* programLayout = m_actualTypeConformance->getLayout(targetIndex, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(programLayout);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt TypeConformanceCapture::getSpecializationParamCount()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualTypeConformance->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_getEntryPointCode, m_typeConformanceHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_getTargetCode, m_typeConformanceHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_getResultAsFileSystem, m_typeConformanceHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            encoder->encodeAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void TypeConformanceCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_getEntryPointHash, m_typeConformanceHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualTypeConformance->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            encoder->encodeAddress(*outHash);
            m_captureManager->endMethodCaptureAppendOutput();
        }
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::specialize(
        slang::SpecializationArg const*     specializationArgs,
        SlangInt                            specializationArgCount,
        slang::IComponentType**             outSpecializedComponentType,
        ISlangBlob**                        outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_specialize, m_typeConformanceHandle);
            encoder->encodeInt64(specializationArgCount);
            encoder->encodeStructArray(specializationArgs, specializationArgCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outSpecializedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_link, m_typeConformanceHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->link(outLinkedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_getEntryPointHostCallable, m_typeConformanceHandle);
            encoder->encodeInt32(entryPointIndex);
            encoder->encodeInt32(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            encoder->encodeAddress(*outSharedLibrary);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_renameEntryPoint, m_typeConformanceHandle);
            encoder->encodeString(newName);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->renameEntryPoint(newName, outEntryPoint);

        {
            encoder->encodeAddress(*outEntryPoint);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ITypeConformance_linkWithOptions, m_typeConformanceHandle);
            encoder->encodeUint32(compilerOptionEntryCount);
            encoder->encodeStructArray(compilerOptionEntries, compilerOptionEntryCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualTypeConformance->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }
}
