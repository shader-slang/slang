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

        // Each method capture has to start with a FunctionHeader
        ParameterEncoder* beginMethodCapture(const ApiCallId& callId, uint64_t handleId);
        ParameterEncoder* endMethodCapture();

        // endMethodCaptureAppendOutput is an optional call that can be used to append output to
        // the end of the capture. It has to start with a FunctionTailer
        void endMethodCaptureAppendOutput();
    private:
        void clearWithHeader(const ApiCallId& callId, uint64_t handleId);
        void clearWithTailer();

        struct FunctionHeader
        {
            uint32_t         magic {0x44414548};
            ApiCallId        callId {InvalidCallId};
            uint64_t         handleId {0};
            uint64_t         dataSizeInBytes {0};
            uint64_t         threadId {0};
        };

        struct FunctionTailer
        {
            uint32_t     magic {0x4C494154};
            uint32_t     dataSizeInBytes {0};
        };

        MemoryStream m_memoryStream;
        std::unique_ptr<FileOutputStream> m_fileStream;
        ParameterEncoder m_encoder;
    };
} // namespace SlangCapture
#endif // CAPTURE_MANAGER_H
