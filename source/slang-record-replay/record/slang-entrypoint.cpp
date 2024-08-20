#include "../util/record-utility.h"
#include "slang-entrypoint.h"

namespace SlangRecord
{
    EntryPointRecorder::EntryPointRecorder(slang::IEntryPoint* entryPoint, RecordManager* recordManager)
        : m_actualEntryPoint(entryPoint),
          m_recordManager(recordManager)
    {
        SLANG_RECORD_ASSERT(m_actualEntryPoint != nullptr);
        SLANG_RECORD_ASSERT(m_recordManager != nullptr);

        m_entryPointHandle = reinterpret_cast<uint64_t>(m_actualEntryPoint.get());
        slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, entryPoint);
    }

    ISlangUnknown* EntryPointRecorder::getInterface(const Guid& guid)
    {
        if(guid == EntryPointRecorder::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    SLANG_NO_THROW slang::ISession* EntryPointRecorder::getSession()
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getSession, m_entryPointHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ISession* session = m_actualEntryPoint->getSession();

        {
            recorder->recordAddress(session);
            m_recordManager->apendOutput();
        }

        return session;
    }

    SLANG_NO_THROW slang::ProgramLayout* EntryPointRecorder::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getLayout, m_entryPointHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ProgramLayout* programLayout = m_actualEntryPoint->getLayout(targetIndex, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(programLayout);
            m_recordManager->apendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt EntryPointRecorder::getSpecializationParamCount()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualEntryPoint->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getEntryPointCode, m_entryPointHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getTargetCode, m_entryPointHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getResultAsFileSystem, m_entryPointHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            recorder->recordAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void EntryPointRecorder::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getEntryPointHash, m_entryPointHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        m_actualEntryPoint->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            recorder->recordAddress(*outHash);
            m_recordManager->apendOutput();
        }
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_specialize, m_entryPointHandle);
            recorder->recordInt64(specializationArgCount);
            recorder->recordStructArray(specializationArgs, specializationArgCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outSpecializedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_link, m_entryPointHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->link(outLinkedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_getEntryPointHostCallable, m_entryPointHandle);
            recorder->recordInt32(entryPointIndex);
            recorder->recordInt32(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            recorder->recordAddress(*outSharedLibrary);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_renameEntryPoint, m_entryPointHandle);
            recorder->recordString(newName);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->renameEntryPoint(newName, outEntryPoint);

        {
            recorder->recordAddress(*outEntryPoint);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointRecorder::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::IEntryPoint_linkWithOptions, m_entryPointHandle);
            recorder->recordUint32(compilerOptionEntryCount);
            recorder->recordStructArray(compilerOptionEntries, compilerOptionEntryCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualEntryPoint->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW slang::FunctionReflection* EntryPointRecorder::getFunctionReflection()
    {
        return m_actualEntryPoint->getFunctionReflection();
    }

}
