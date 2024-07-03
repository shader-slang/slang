#ifndef CAPTURE_MANAGER_H
#define CAPTURE_MANAGER_H

#include <filesystem>
#include "parameter-encoder.h"
#include "../capture-format.h"

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

        std::filesystem::path const& getCaptureFileDirectory() const { return m_captureFileDirectory; }

    private:
        void clearWithHeader(const ApiCallId& callId, uint64_t handleId);
        void clearWithTailer();

        MemoryStream m_memoryStream;
        std::unique_ptr<FileOutputStream> m_fileStream;
        std::filesystem::path             m_captureFileDirectory = std::filesystem::current_path();
        ParameterEncoder m_encoder;
    };
} // namespace SlangCapture
#endif // CAPTURE_MANAGER_H
