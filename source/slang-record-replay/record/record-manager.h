#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include <filesystem>
#include "parameter-recorder.h"
#include "../util/record-format.h"

namespace SlangRecord
{
    class RecordManager
    {
    public:
        RecordManager(uint64_t globalSessionHandle);

        // Each method record has to start with a FunctionHeader
        ParameterRecorder* beginMethodRecord(const ApiCallId& callId, uint64_t handleId);
        ParameterRecorder* endMethodRecord();

        // endMethodRecordAppendOutput is an optional call that can be used to append output to
        // the end of the record. It has to start with a FunctionTailer
        void endMethodRecordAppendOutput();

        std::filesystem::path const& getRecordFileDirectory() const { return m_recordFileDirectory; }

    private:
        void clearWithHeader(const ApiCallId& callId, uint64_t handleId);
        void clearWithTailer();

        MemoryStream m_memoryStream;
        std::unique_ptr<FileOutputStream> m_fileStream;
        std::filesystem::path             m_recordFileDirectory = std::filesystem::current_path();
        ParameterRecorder m_recorder;
    };
} // namespace SlangRecord
#endif // RECORD_MANAGER_H
