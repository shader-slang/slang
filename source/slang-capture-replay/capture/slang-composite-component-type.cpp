#include "../capture-utility.h"
#include "slang-composite-component-type.h"

namespace SlangCapture
{
    CompositeComponentTypeCapture::CompositeComponentTypeCapture(
            slang::IComponentType* componentType, CaptureManager* captureManager)
        : m_actualCompositeComponentType(componentType),
          m_captureManager(captureManager)
    {
        SLANG_CAPTURE_ASSERT(m_actualCompositeComponentType != nullptr);
        SLANG_CAPTURE_ASSERT(m_captureManager != nullptr);

        m_compositeComponentHandle = reinterpret_cast<uint64_t>(m_actualCompositeComponentType.get());
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, componentType);
    }

    CompositeComponentTypeCapture::~CompositeComponentTypeCapture()
    {
        m_actualCompositeComponentType->release();
    }

    ISlangUnknown* CompositeComponentTypeCapture::getInterface(const Guid& guid)
    {
        if (guid == IComponentType::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        return nullptr;
    }

    SLANG_NO_THROW slang::ISession* CompositeComponentTypeCapture::getSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getSession, m_compositeComponentHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ISession* res = m_actualCompositeComponentType->getSession();

        {
            encoder->encodeAddress(res);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* CompositeComponentTypeCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getLayout, m_compositeComponentHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ProgramLayout* programLayout = m_actualCompositeComponentType->getLayout(targetIndex, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(programLayout);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt CompositeComponentTypeCapture::getSpecializationParamCount()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualCompositeComponentType->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getEntryPointCode, m_compositeComponentHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getTargetCode, m_compositeComponentHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getResultAsFileSystem, m_compositeComponentHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            encoder->encodeAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void CompositeComponentTypeCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getEntryPointHash, m_compositeComponentHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualCompositeComponentType->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            encoder->encodeAddress(*outHash);
            m_captureManager->endMethodCaptureAppendOutput();
        }
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_specialize, m_compositeComponentHandle);
            encoder->encodeInt64(specializationArgCount);
            encoder->encodeStructArray(specializationArgs, specializationArgCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outSpecializedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_link, m_compositeComponentHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->link(outLinkedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_getEntryPointHostCallable, m_compositeComponentHandle);
            encoder->encodeInt32(entryPointIndex);
            encoder->encodeInt32(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            encoder->encodeAddress(*outSharedLibrary);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_renameEntryPoint, m_compositeComponentHandle);
            encoder->encodeString(newName);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->renameEntryPoint(newName, outEntryPoint);

        {
            encoder->encodeAddress(*outEntryPoint);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ICompositeComponentType_linkWithOptions, m_compositeComponentHandle);
            encoder->encodeUint32(compilerOptionEntryCount);
            encoder->encodeStructArray(compilerOptionEntries, compilerOptionEntryCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualCompositeComponentType->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }
}
