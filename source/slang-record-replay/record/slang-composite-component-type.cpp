#include "../util/record-utility.h"
#include "slang-composite-component-type.h"

namespace SlangRecord
{
    CompositeComponentTypeRecorder::CompositeComponentTypeRecorder(
            slang::IComponentType* componentType, RecordManager* recordManager)
        : m_actualCompositeComponentType(componentType),
          m_recordManager(recordManager)
    {
        SLANG_RECORD_ASSERT(m_actualCompositeComponentType != nullptr);
        SLANG_RECORD_ASSERT(m_recordManager != nullptr);

        m_compositeComponentHandle = reinterpret_cast<uint64_t>(m_actualCompositeComponentType.get());
        slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, componentType);
    }

    ISlangUnknown* CompositeComponentTypeRecorder::getInterface(const Guid& guid)
    {
        if (guid == IComponentType::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        return nullptr;
    }

    SLANG_NO_THROW slang::ISession* CompositeComponentTypeRecorder::getSession()
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getSession, m_compositeComponentHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ISession* res = m_actualCompositeComponentType->getSession();

        {
            recorder->recordAddress(res);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* CompositeComponentTypeRecorder::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getLayout, m_compositeComponentHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ProgramLayout* programLayout = m_actualCompositeComponentType->getLayout(targetIndex, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(programLayout);
            m_recordManager->apendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt CompositeComponentTypeRecorder::getSpecializationParamCount()
    {
        // No need to record this call as it is just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualCompositeComponentType->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getEntryPointCode, m_compositeComponentHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getTargetCode, m_compositeComponentHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getResultAsFileSystem, m_compositeComponentHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            recorder->recordAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void CompositeComponentTypeRecorder::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getEntryPointHash, m_compositeComponentHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        m_actualCompositeComponentType->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            recorder->recordAddress(*outHash);
            m_recordManager->apendOutput();
        }
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_specialize, m_compositeComponentHandle);
            recorder->recordInt64(specializationArgCount);
            recorder->recordStructArray(specializationArgs, specializationArgCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outSpecializedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_link, m_compositeComponentHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->link(outLinkedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getEntryPointHostCallable, m_compositeComponentHandle);
            recorder->recordInt32(entryPointIndex);
            recorder->recordInt32(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            recorder->recordAddress(*outSharedLibrary);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_renameEntryPoint, m_compositeComponentHandle);
            recorder->recordString(newName);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->renameEntryPoint(newName, outEntryPoint);

        {
            recorder->recordAddress(*outEntryPoint);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeRecorder::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_linkWithOptions, m_compositeComponentHandle);
            recorder->recordUint32(compilerOptionEntryCount);
            recorder->recordStructArray(compilerOptionEntries, compilerOptionEntryCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualCompositeComponentType->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }
}
