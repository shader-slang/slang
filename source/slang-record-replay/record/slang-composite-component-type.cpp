#include "../util/record-utility.h"
#include "slang-composite-component-type.h"

namespace SlangRecord
{
    CompositeComponentTypeRecorder::CompositeComponentTypeRecorder(
            slang::IComponentType* componentType, RecordManager* recordManager)
        : IComponentTypeRecorder(componentType, recordManager)
    {
        slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, componentType);
    }

    ISlangUnknown* CompositeComponentTypeRecorder::getInterface(const Guid& guid)
    {
        if (guid == CompositeComponentTypeRecorder::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        return nullptr;
    }
}
