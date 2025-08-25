#include "slang-composite-component-type.h"

#include "../util/record-utility.h"

namespace SlangRecord
{
CompositeComponentTypeRecorder::CompositeComponentTypeRecorder(
    SessionRecorder* sessionRecorder,
    slang::IComponentType* componentType,
    RecordManager* recordManager)
    : IComponentTypeRecorder(componentType, recordManager), m_sessionRecorder(sessionRecorder)
{
    slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, componentType);
}

ISlangUnknown* CompositeComponentTypeRecorder::getInterface(const Guid& guid)
{
    // Record the queryInterface call
    ApiCallId callId =
        static_cast<ApiCallId>(makeApiCallId(getClassId(), IComponentTypeMethodId::queryInterface));
    ParameterRecorder* recorder{};
    {
        recorder = m_recordManager->beginMethodRecord(callId, m_componentHandle);
        recorder->recordGuid(guid);
        recorder = m_recordManager->endMethodRecord();
    }

    ISlangUnknown* result = nullptr;
    if (guid == CompositeComponentTypeRecorder::getTypeGuid())
    {
        result = static_cast<ISlangUnknown*>(this);
    }
    else
    {
        // Delegate to the base class for IComponentType2 support.
        result = IComponentTypeRecorder::getInterface(guid);
    }

    // Record the result
    {
        recorder->recordAddress(result);
        m_recordManager->apendOutput();
    }

    return result;
}
} // namespace SlangRecord
