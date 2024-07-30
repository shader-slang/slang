#include "../util/record-utility.h"
#include "slang-type-conformance.h"

namespace SlangRecord
{
    TypeConformanceRecorder::TypeConformanceRecorder(slang::ITypeConformance* typeConformance, RecordManager* recordManager)
        : m_actualTypeConformance(typeConformance),
          m_recordManager(recordManager)
    {
        SLANG_RECORD_ASSERT(m_actualTypeConformance != nullptr);
        SLANG_RECORD_ASSERT(m_recordManager != nullptr);

        m_typeConformanceHandle = reinterpret_cast<uint64_t>(m_actualTypeConformance.get());
        slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, typeConformance);
    }

    TypeConformanceRecorder::~TypeConformanceRecorder()
    {
        m_actualTypeConformance->release();
    }

    ISlangUnknown* TypeConformanceRecorder::getInterface(const Guid& guid)
    {
        if (guid == TypeConformanceRecorder::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        else
        {
            return nullptr;
        }
    }

    SLANG_NO_THROW slang::ISession* TypeConformanceRecorder::getSession()
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_getSession, m_typeConformanceHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ISession* res = m_actualTypeConformance->getSession();

        {
            recorder->recordAddress(res);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* TypeConformanceRecorder::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ICompositeComponentType_getLayout, m_typeConformanceHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::ProgramLayout* programLayout = m_actualTypeConformance->getLayout(targetIndex, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(programLayout);
            m_recordManager->apendOutput();
        }

        return programLayout;
    }

    SLANG_NO_THROW SlangInt TypeConformanceRecorder::getSpecializationParamCount()
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualTypeConformance->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_getEntryPointCode, m_typeConformanceHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_getTargetCode, m_typeConformanceHandle);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->getTargetCode(targetIndex, outCode, outDiagnostics);

        {
            recorder->recordAddress(*outCode);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_getResultAsFileSystem, m_typeConformanceHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);

        {
            recorder->recordAddress(*outFileSystem);
        }

        // TODO: We might need to wrap the file system object.
        return res;
    }

    SLANG_NO_THROW void TypeConformanceRecorder::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_getEntryPointHash, m_typeConformanceHandle);
            recorder->recordInt64(entryPointIndex);
            recorder->recordInt64(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        m_actualTypeConformance->getEntryPointHash(entryPointIndex, targetIndex, outHash);

        {
            recorder->recordAddress(*outHash);
            m_recordManager->apendOutput();
        }
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::specialize(
        slang::SpecializationArg const*     specializationArgs,
        SlangInt                            specializationArgCount,
        slang::IComponentType**             outSpecializedComponentType,
        ISlangBlob**                        outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_specialize, m_typeConformanceHandle);
            recorder->recordInt64(specializationArgCount);
            recorder->recordStructArray(specializationArgs, specializationArgCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outSpecializedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_link, m_typeConformanceHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->link(outLinkedComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_getEntryPointHostCallable, m_typeConformanceHandle);
            recorder->recordInt32(entryPointIndex);
            recorder->recordInt32(targetIndex);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);

        {
            recorder->recordAddress(*outSharedLibrary);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_renameEntryPoint, m_typeConformanceHandle);
            recorder->recordString(newName);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->renameEntryPoint(newName, outEntryPoint);

        {
            recorder->recordAddress(*outEntryPoint);
            m_recordManager->apendOutput();
        }

        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceRecorder::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ITypeConformance_linkWithOptions, m_typeConformanceHandle);
            recorder->recordUint32(compilerOptionEntryCount);
            recorder->recordStructArray(compilerOptionEntries, compilerOptionEntryCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult res = m_actualTypeConformance->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);

        {
            recorder->recordAddress(*outLinkedComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        return res;
    }
}
