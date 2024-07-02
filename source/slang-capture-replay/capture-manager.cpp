
#include <string>
#include <sstream>
#include <thread>
#include "capture-utility.h"
#include "capture-manager.h"

namespace SlangCapture
{
    CaptureManager::CaptureManager(uint64_t globalSessionHandle)
        : m_encoder(&m_memoryStream)
    {
        std::stringstream ss;
        ss << "gs-"<< globalSessionHandle <<"-t-"<<std::this_thread::get_id() << ".cap";

        m_captureFileDirectory = m_captureFileDirectory / "slang-capture";

        if (!std::filesystem::exists(m_captureFileDirectory))
        {
            std::error_code ec;
            if (!std::filesystem::create_directory(m_captureFileDirectory, ec))
            {
                slangCaptureLog(LogLevel::Error, "Fail to create directory: %s, error (%d): %s\n",
                    m_captureFileDirectory.string().c_str(), ec.value(), ec.message().c_str());
            }
        }

        std::filesystem::path captureFilePath = m_captureFileDirectory / ss.str();
        m_fileStream = std::make_unique<FileOutputStream>(captureFilePath.string());
    }

    void CaptureManager::clearWithHeader(const ApiCallId& callId, uint64_t handleId)
    {
        m_memoryStream.flush();
        FunctionHeader header;
        header.callId = callId;
        header.handleId = handleId;

        // write header to memory stream
        m_memoryStream.write(&header, sizeof(FunctionHeader));
    }

    void CaptureManager::clearWithTailer()
    {
        m_memoryStream.flush();
        FunctionTailer tailer;

        // write header to memory stream
        m_memoryStream.write(&tailer, sizeof(FunctionTailer));
    }

    ParameterEncoder* CaptureManager::beginMethodCapture(const ApiCallId& callId, uint64_t handleId)
    {
        clearWithHeader(callId, handleId);
        return &m_encoder;
    }

    ParameterEncoder* CaptureManager::endMethodCapture()
    {
        FunctionHeader* pHeader = const_cast<FunctionHeader*>(
                reinterpret_cast<const FunctionHeader*>(m_memoryStream.getData()));

        pHeader->dataSizeInBytes = m_memoryStream.getSizeInBytes() - sizeof(FunctionHeader);

        std::hash<std::thread::id> hasher;
        pHeader->threadId = hasher(std::this_thread::get_id());

        // write capture data to file
        m_fileStream->write(m_memoryStream.getData(), m_memoryStream.getSizeInBytes());

        // take effect of the write
        m_fileStream->flush();

        // clear the memory stream
        m_memoryStream.flush();

        clearWithTailer();
        return &m_encoder;
    }

    void CaptureManager::endMethodCaptureAppendOutput()
    {
        FunctionTailer* pTailer = const_cast<FunctionTailer*>(
                reinterpret_cast<const FunctionTailer*>(m_memoryStream.getData()));

        pTailer->dataSizeInBytes = (uint32_t)(m_memoryStream.getSizeInBytes() - sizeof(FunctionTailer));

        // write capture data to file
        m_fileStream->write(m_memoryStream.getData(), m_memoryStream.getSizeInBytes());

        // take effect of the write
        m_fileStream->flush();

        // clear the memory stream
        m_memoryStream.flush();
    }
}
