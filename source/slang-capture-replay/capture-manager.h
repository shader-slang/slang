#ifndef CAPTURE_MANAGER_H
#define CAPTURE_MANAGER_H

#include "parameter-encoder.h"
#include "api_callId.h"
namespace SlangCapture
{
    class CaptureManager
    {
    public:
        CaptureManager(uint64_t globalSessionHandle);
        ParameterEncoder* beginMethodCapture(const ApiCallId& callId, uint64_t handleId);
        void endMethodCapture();
    private:
        void clearWithHeader(const ApiCallId& callId, uint64_t handleId);

        struct FunctionHeader
        {
            ApiCallId        callId {InvalidCallId};
            uint64_t         handleId {0};
            uint64_t         dataSizeInBytes {0};
            uint64_t         threadId {0};
        };

        MemoryStream m_memoryStream;
        std::unique_ptr<FileOutputStream> m_fileStream;
        ParameterEncoder m_encoder;
    };
} // namespace SlangCapture
#endif // CAPTURE_MANAGER_H
