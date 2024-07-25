
#include <string>
#include <sstream>
#include <thread>
#include "../util/record-utility.h"
#include "record-manager.h"

namespace SlangRecord
{
    RecordManager::RecordManager(uint64_t globalSessionHandle)
        : m_recorder(&m_memoryStream)
    {
        std::stringstream ss;
        ss << "gs-"<< globalSessionHandle <<"-t-"<<std::this_thread::get_id() << ".cap";

        m_recordFileDirectory = m_recordFileDirectory / "slang-record";

        if (!std::filesystem::exists(m_recordFileDirectory))
        {
            std::error_code ec;
            if (!std::filesystem::create_directory(m_recordFileDirectory, ec))
            {
                slangRecordLog(LogLevel::Error, "Fail to create directory: %s, error (%d): %s\n",
                    m_recordFileDirectory.string().c_str(), ec.value(), ec.message().c_str());
            }
        }

        std::filesystem::path recordFilePath = m_recordFileDirectory / ss.str();
        m_fileStream = std::make_unique<FileOutputStream>(recordFilePath.string());
    }

    void RecordManager::clearWithHeader(const ApiCallId& callId, uint64_t handleId)
    {
        m_memoryStream.flush();
        FunctionHeader header;
        header.callId = callId;
        header.handleId = handleId;

        // write header to memory stream
        m_memoryStream.write(&header, sizeof(FunctionHeader));
    }

    void RecordManager::clearWithTailer()
    {
        m_memoryStream.flush();
        FunctionTailer tailer;

        // write header to memory stream
        m_memoryStream.write(&tailer, sizeof(FunctionTailer));
    }

    ParameterRecorder* RecordManager::beginMethodRecord(const ApiCallId& callId, uint64_t handleId)
    {
        clearWithHeader(callId, handleId);
        return &m_recorder;
    }

    ParameterRecorder* RecordManager::endMethodRecord()
    {
        FunctionHeader* pHeader = const_cast<FunctionHeader*>(
                reinterpret_cast<const FunctionHeader*>(m_memoryStream.getData()));

        pHeader->dataSizeInBytes = m_memoryStream.getSizeInBytes() - sizeof(FunctionHeader);

        std::hash<std::thread::id> hasher;
        pHeader->threadId = hasher(std::this_thread::get_id());

        // write record data to file
        m_fileStream->write(m_memoryStream.getData(), m_memoryStream.getSizeInBytes());

        // take effect of the write
        m_fileStream->flush();

        // clear the memory stream
        m_memoryStream.flush();

        clearWithTailer();
        return &m_recorder;
    }

    void RecordManager::endMethodRecordAppendOutput()
    {
        FunctionTailer* pTailer = const_cast<FunctionTailer*>(
                reinterpret_cast<const FunctionTailer*>(m_memoryStream.getData()));

        pTailer->dataSizeInBytes = (uint32_t)(m_memoryStream.getSizeInBytes() - sizeof(FunctionTailer));

        // write record data to file
        m_fileStream->write(m_memoryStream.getData(), m_memoryStream.getSizeInBytes());

        // take effect of the write
        m_fileStream->flush();

        // clear the memory stream
        m_memoryStream.flush();
    }
}
