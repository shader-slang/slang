#include "capture_utility.h"
#include "slang-module.h"

namespace SlangCapture
{
    ModuleCapture::ModuleCapture(slang::IModule* module, CaptureManager* captureManager)
        : m_actualModule(module),
          m_captureManager(captureManager)
    {
        SLANG_CAPTURE_ASSERT(m_actualModule != nullptr);
        SLANG_CAPTURE_ASSERT(m_captureManager != nullptr);

        m_moduleHandle = reinterpret_cast<uint64_t>(m_actualModule.get());
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, module);
    }

    ModuleCapture::~ModuleCapture()
    {
        m_actualModule->release();
    }

    ISlangUnknown* ModuleCapture::getInterface(const Guid& guid)
    {
        if(guid == ModuleCapture::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::findEntryPointByName(
        char const*     name,
        slang::IEntryPoint**   outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_findEntryPointByName, m_moduleHandle);
            encoder->encodeString(name);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->findEntryPointByName(name, outEntryPoint);

        {
            encoder->encodeAddress(*outEntryPoint);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (SLANG_OK == res)
        {
            EntryPointCapture* entryPointCapture = getEntryPointCapture(*outEntryPoint);
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointCapture);
        }
        return res;
    }

    SLANG_NO_THROW SlangInt32 ModuleCapture::getDefinedEntryPointCount()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt32 res = m_actualModule->getDefinedEntryPointCount();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getDefinedEntryPoint(SlangInt32 index, slang::IEntryPoint** outEntryPoint)
    {
        // This call is to find the existing entry point, so it has been created already. Therefore, we don't create a new one
        // and assert the error if it is not found in our map.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getDefinedEntryPoint, m_moduleHandle);
            encoder->encodeInt32(index);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->getDefinedEntryPoint(index, outEntryPoint);

        {
            encoder->encodeAddress(*outEntryPoint);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (*outEntryPoint)
        {
            EntryPointCapture* entryPointCapture = m_mapEntryPointToCapture.tryGetValue(*outEntryPoint);
            if (!entryPointCapture)
            {
                SLANG_CAPTURE_ASSERT(!"Entrypoint not found in mapEntryPointToCapture");
            }
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointCapture);
        }
        else
            *outEntryPoint = nullptr;

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::serialize(ISlangBlob** outSerializedBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_serialize, m_moduleHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->serialize(outSerializedBlob);

        {
            encoder->encodeAddress(*outSerializedBlob);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::writeToFile(char const* fileName)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_writeToFile, m_moduleHandle);
            encoder->encodeString(fileName);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->writeToFile(fileName);
        return res;
    }

    SLANG_NO_THROW const char* ModuleCapture::getName()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getName();
        return res;
    }

    SLANG_NO_THROW const char* ModuleCapture::getFilePath()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getFilePath();
        return res;
    }

    SLANG_NO_THROW const char* ModuleCapture::getUniqueIdentity()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getUniqueIdentity();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::findAndCheckEntryPoint(
        char const* name,
        SlangStage stage,
        slang::IEntryPoint** outEntryPoint,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_findAndCheckEntryPoint, m_moduleHandle);
            encoder->encodeString(name);
            encoder->encodeEnumValue(stage);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->findAndCheckEntryPoint(name, stage, outEntryPoint, outDiagnostics);

        {
            encoder->encodeAddress(*outEntryPoint);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (SLANG_OK == res)
        {
            EntryPointCapture* entryPointCapture = getEntryPointCapture(*outEntryPoint);
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointCapture);
        }
        return res;
    }

    SLANG_NO_THROW slang::ISession* ModuleCapture::getSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getSession, m_moduleHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ISession* session = m_actualModule->getSession();

        {
            encoder->encodeAddress(session);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return session;
    }

    SLANG_NO_THROW slang::ProgramLayout* ModuleCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getLayout, m_moduleHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::ProgramLayout* programLayout = m_actualModule->getLayout(targetIndex, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(programLayout);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt ModuleCapture::getSpecializationParamCount()
    {
        // No need to capture this call as it is just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualModule->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getEntryPointCode, m_moduleHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getTargetCode, m_moduleHandle);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            encoder->encodeAddress(*outCode);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getResultAsFileSystem, m_moduleHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            encoder->encodeAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void ModuleCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getEntryPointHash, m_moduleHandle);
            encoder->encodeInt64(entryPointIndex);
            encoder->encodeInt64(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        m_actualModule->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            encoder->encodeAddress(*outHash);
            m_captureManager->endMethodCaptureAppendOutput();
        }
    }

    SLANG_NO_THROW SlangResult ModuleCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_specialize, m_moduleHandle);
            encoder->encodeInt64(specializationArgCount);
            encoder->encodeStructArray(specializationArgs, specializationArgCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outSpecializedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::link(
        IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_link, m_moduleHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->link(outLinkedComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_getEntryPointHostCallable, m_moduleHandle);
            encoder->encodeInt32(entryPointIndex);
            encoder->encodeInt32(targetIndex);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            encoder->encodeAddress(*outSharedLibrary);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_renameEntryPoint, m_moduleHandle);
            encoder->encodeString(newName);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->renameEntryPoint(newName, outEntryPoint);

        {
            encoder->encodeAddress(*outEntryPoint);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::IModule_linkWithOptions, m_moduleHandle);
            encoder->encodeUint32(compilerOptionEntryCount);
            encoder->encodeStructArray(compilerOptionEntries, compilerOptionEntryCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult res = m_actualModule->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            encoder->encodeAddress(*outLinkedComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return res;
    }

    EntryPointCapture* ModuleCapture::getEntryPointCapture(slang::IEntryPoint* entryPoint)
    {
        EntryPointCapture* entryPointCapture = nullptr;
        entryPointCapture = m_mapEntryPointToCapture.tryGetValue(entryPoint);
        if (!entryPointCapture)
        {
            entryPointCapture = new EntryPointCapture(entryPoint, m_captureManager);
            Slang::ComPtr<EntryPointCapture> result(entryPointCapture);
            m_mapEntryPointToCapture.add(entryPoint, *result.detach());
        }
        return entryPointCapture;
    }
}
