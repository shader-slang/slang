
#include <string>
#include <sstream>
#include <thread>
#include "capture-manager.h"

namespace SlangCapture
{
    CaptureManager::CaptureManager(uint64_t globalSessionHandle)
        : m_encoder(&m_memoryStream)
    {
        std::stringstream ss;
        ss << "gs-"<< globalSessionHandle <<"t-"<<std::this_thread::get_id() << ".cap";
        m_fileStream = std::make_unique<FileOutputStream>(ss.str());
    }

    void CaptureManager::clearWithHeader(const ApiCallId& callId, uint64_t handleId)
    {
        m_memoryStream.flush();
        FunctionHeader header {};
        header.callId = callId;
        header.handleId = handleId;

        // write header to memory stream
        m_memoryStream.write(&header, sizeof(FunctionHeader));
    }

    ParameterEncoder* CaptureManager::beginMethodCapture(const ApiCallId& callId, uint64_t handleId)
    {
        clearWithHeader(callId, handleId);
        return &m_encoder;
    }

    void CaptureManager::endMethodCapture()
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
    }
}
