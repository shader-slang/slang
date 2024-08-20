#include "../util/record-utility.h"
#include "slang-module.h"

namespace SlangRecord
{
    ModuleRecorder::ModuleRecorder(slang::IModule* module, RecordManager* recordManager)
        : m_actualModule(module),
          m_recordManager(recordManager)
    {
        SLANG_RECORD_ASSERT(m_actualModule != nullptr);
        SLANG_RECORD_ASSERT(m_recordManager != nullptr);

        m_moduleHandle = reinterpret_cast<uint64_t>(m_actualModule.get());
        slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, module);
    }

    ISlangUnknown* ModuleRecorder::getInterface(const Guid& guid)
    {
        if(guid == ModuleRecorder::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    SLANG_NO_THROW slang::DeclReflection* ModuleRecorder::getModuleReflection()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::DeclReflection* res = (slang::DeclReflection*)m_actualModule->getModuleReflection();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::findEntryPointByName(
        char const*     name,
        slang::IEntryPoint**   outEntryPoint)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_findEntryPointByName, m_moduleHandle);
            recorder->recordString(name);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->findEntryPointByName(name, outEntryPoint);

        {
            recorder->recordAddress(*outEntryPoint);
            m_recordManager->apendOutput();
        }

        if (SLANG_OK == res)
        {
            EntryPointRecorder* entryPointRecord = getEntryPointRecorder(*outEntryPoint);
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointRecord);
        }
        return res;
    }

    SLANG_NO_THROW SlangInt32 ModuleRecorder::getDefinedEntryPointCount()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt32 res = m_actualModule->getDefinedEntryPointCount();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::getDefinedEntryPoint(SlangInt32 index, slang::IEntryPoint** outEntryPoint)
    {
        // This call is to find the existing entry point, so it has been created already. Therefore, we don't create a new one
        // and assert the error if it is not found in our map.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getDefinedEntryPoint, m_moduleHandle);
            recorder->recordInt32(index);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->getDefinedEntryPoint(index, outEntryPoint);

        {
            recorder->recordAddress(*outEntryPoint);
            m_recordManager->apendOutput();
        }

        if (*outEntryPoint)
        {
            EntryPointRecorder* entryPointRecord = m_mapEntryPointToRecord.tryGetValue(*outEntryPoint);
            if (!entryPointRecord)
            {
                SLANG_RECORD_ASSERT(!"Entrypoint not found in mapEntryPointToRecord");
            }
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointRecord);
        }
        else
            *outEntryPoint = nullptr;

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::serialize(ISlangBlob** outSerializedBlob)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_serialize, m_moduleHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->serialize(outSerializedBlob);

        {
            recorder->recordAddress(*outSerializedBlob);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::writeToFile(char const* fileName)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_writeToFile, m_moduleHandle);
            recorder->recordString(fileName);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->writeToFile(fileName);
        return res;
    }

    SLANG_NO_THROW const char* ModuleRecorder::getName()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getName();
        return res;
    }

    SLANG_NO_THROW const char* ModuleRecorder::getFilePath()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getFilePath();
        return res;
    }

    SLANG_NO_THROW const char* ModuleRecorder::getUniqueIdentity()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getUniqueIdentity();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::findAndCheckEntryPoint(
        char const* name,
        SlangStage stage,
        slang::IEntryPoint** outEntryPoint,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_findAndCheckEntryPoint, m_moduleHandle);
            recorder->recordString(name);
            recorder->recordEnumValue(stage);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->findAndCheckEntryPoint(name, stage, outEntryPoint, outDiagnostics);

        {
            recorder->recordAddress(*outEntryPoint);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        if (SLANG_OK == res)
        {
            EntryPointRecorder* entryPointRecord = getEntryPointRecorder(*outEntryPoint);
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointRecord);
        }
        return res;
    }

    SLANG_NO_THROW SlangInt32 ModuleRecorder::getDependencyFileCount()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt32 res = m_actualModule->getDependencyFileCount();
        return res;
    }

    SLANG_NO_THROW char const* ModuleRecorder::getDependencyFilePath(SlangInt32 index)
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getDependencyFilePath(index);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::precompileForTarget(
        SlangCompileTarget target,
        ISlangBlob** outDiagnostics)
    {
        // TODO: We should record this call
        // https://github.com/shader-slang/slang/issues/4853
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->precompileForTarget(target, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW slang::ISession* ModuleRecorder::getSession()
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        slang::ISession* session = m_actualModule->getSession();

        return session;
    }

    SLANG_NO_THROW slang::ProgramLayout* ModuleRecorder::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getLayout, m_moduleHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ProgramLayout* programLayout = m_actualModule->getLayout(targetIndex, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(programLayout);
            m_recordManager->apendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt ModuleRecorder::getSpecializationParamCount()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualModule->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getEntryPointCode, m_moduleHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getTargetCode, m_moduleHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getResultAsFileSystem, m_moduleHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            recorder->recordAddress(*outFileSystem);
            m_recordManager->apendOutput();
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void ModuleRecorder::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getEntryPointHash, m_moduleHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        m_actualModule->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            recorder->recordAddress(*outHash);
            m_recordManager->apendOutput();
        }
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_specialize, m_moduleHandle);
            recorder->recordInt64(specializationArgCount);
            recorder->recordStructArray(specializationArgs, specializationArgCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outSpecializedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::link(
        IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_link, m_moduleHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->link(outLinkedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_getEntryPointHostCallable, m_moduleHandle);
            recorder->recordInt32(entryPointIndex);
            recorder->recordInt32(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            recorder->recordAddress(*outSharedLibrary);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_renameEntryPoint, m_moduleHandle);
            recorder->recordString(newName);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->renameEntryPoint(newName, outEntryPoint);

        {
            recorder->recordAddress(*outEntryPoint);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleRecorder::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IModule_linkWithOptions, m_moduleHandle);
            recorder->recordUint32(compilerOptionEntryCount);
            recorder->recordStructArray(compilerOptionEntries, compilerOptionEntryCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualModule->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    EntryPointRecorder* ModuleRecorder::getEntryPointRecorder(slang::IEntryPoint* entryPoint)
    {
        EntryPointRecorder* entryPointRecord = nullptr;
        entryPointRecord = m_mapEntryPointToRecord.tryGetValue(entryPoint);
        if (!entryPointRecord)
        {
            entryPointRecord = new EntryPointRecorder(entryPoint, m_recordManager);
            Slang::ComPtr<EntryPointRecorder> result(entryPointRecord);
            m_mapEntryPointToRecord.add(entryPoint, *result.detach());
        }
        return entryPointRecord;
    }
}
