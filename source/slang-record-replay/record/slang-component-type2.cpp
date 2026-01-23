#include "slang-component-type2.h"

#include "../util/record-utility.h"
#include "slang-composite-component-type.h"
#include "slang-session.h"

namespace SlangRecord
{
IComponentType2Recorder::IComponentType2Recorder(
    slang::IComponentType2* componentType,
    RecordManager* recordManager)
    : m_actualComponentType2(componentType), m_recordManager(recordManager)
{
    SLANG_RECORD_ASSERT(m_actualComponentType2 != nullptr);
    SLANG_RECORD_ASSERT(m_recordManager != nullptr);

    m_componentType2Handle = reinterpret_cast<uint64_t>(m_actualComponentType2.get());
    slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, componentType);
}

ISlangUnknown* IComponentType2Recorder::getInterface(const Guid& guid)
{
    if (guid == IComponentType2Recorder::getTypeGuid())
        return static_cast<IComponentType2Recorder*>(this);
    else
        return nullptr;
}

SLANG_NO_THROW SlangResult IComponentType2Recorder::getTargetHostCallable(
    int targetIndex,
    ISlangSharedLibrary** outSharedLibrary,
    slang::IBlob** outDiagnostics)
{
    slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

    ApiCallId callId = static_cast<ApiCallId>(
        makeApiCallId(getClassId(), IComponentTypeMethodId::getTargetHostCallable));
    ParameterRecorder* recorder{};
    {
        recorder = m_recordManager->beginMethodRecord(callId, m_componentType2Handle);
        recorder->recordInt32(targetIndex);
        recorder = m_recordManager->endMethodRecord();
    }

    SlangResult res = m_actualComponentType2->getTargetHostCallable(
        targetIndex,
        outSharedLibrary,
        outDiagnostics);

    {
        recorder->recordAddress(*outSharedLibrary);
        recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
        m_recordManager->apendOutput();
    }

    return res;
}

SlangResult IComponentType2Recorder::getTargetCompileResult(
    SlangInt targetIndex,
    slang::ICompileResult** outCompileResult,
    slang::IBlob** outDiagnostics)
{
    slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

    ApiCallId callId = static_cast<ApiCallId>(
        makeApiCallId(getClassId(), IComponentTypeMethodId::getTargetCompileResult));
    ParameterRecorder* recorder{};
    {
        recorder = m_recordManager->beginMethodRecord(callId, m_componentType2Handle);
        recorder->recordInt64(targetIndex);
        recorder = m_recordManager->endMethodRecord();
    }

    SlangResult res = m_actualComponentType2->getTargetCompileResult(
        targetIndex,
        outCompileResult,
        outDiagnostics);
    {
        recorder->recordAddress(*outCompileResult);
        recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
        m_recordManager->apendOutput();
    }

    return res;
}

SlangResult IComponentType2Recorder::getEntryPointCompileResult(
    SlangInt entryPointIndex,
    SlangInt targetIndex,
    slang::ICompileResult** outCompileResult,
    slang::IBlob** outDiagnostics)
{
    slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

    ApiCallId callId = static_cast<ApiCallId>(
        makeApiCallId(getClassId(), IComponentTypeMethodId::getEntryPointCompileResult));
    ParameterRecorder* recorder{};
    {
        recorder = m_recordManager->beginMethodRecord(callId, m_componentType2Handle);
        recorder->recordInt64(entryPointIndex);
        recorder->recordInt64(targetIndex);
        recorder = m_recordManager->endMethodRecord();
    }

    SlangResult res = m_actualComponentType2->getEntryPointCompileResult(
        entryPointIndex,
        targetIndex,
        outCompileResult,
        outDiagnostics);

    {
        recorder->recordAddress(*outCompileResult);
        recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
        m_recordManager->apendOutput();
    }

    return res;
}

} // namespace SlangRecord
