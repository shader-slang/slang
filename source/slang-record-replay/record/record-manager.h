#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include "parameter-recorder.h"
#include "../util/record-format.h"

#include "../../core/slang-string.h"
#include "../../core/slang-io.h"

namespace SlangRecord
{
    class RecordManager
    {
    public:
        RecordManager(uint64_t globalSessionHandle);

        // Each method record has to start with a FunctionHeader
        ParameterRecorder* beginMethodRecord(const ApiCallId& callId, uint64_t handleId);
        ParameterRecorder* endMethodRecord();

        // apendOutput is an optional call that can be used to append output to
        // the end of the record. It has to start with a FunctionTailer
        void apendOutput();

        const Slang::String& getRecordFileDirectory() const { return m_recordFileDirectory; }

    private:
        void clearWithHeader(const ApiCallId& callId, uint64_t handleId);
        void clearWithTailer();

        MemoryStream m_memoryStream;
        std::unique_ptr<FileOutputStream> m_fileStream;
        Slang::String m_recordFileDirectory = Slang::Path::getCurrentPath();
        ParameterRecorder m_recorder;
    };
} // namespace SlangRecord
#endif // RECORD_MANAGER_H
