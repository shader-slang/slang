#include "captureFile-processor.h"
#include "../util/capture-utility.h"
#include "parameter-decoder.h"

namespace SlangCapture
{
    CaptureFileProcessor::CaptureFileProcessor(const std::string& filename)
    {
        Slang::String path(filename.c_str());
        Slang::FileMode fileMode = Slang::FileMode::Open;
        Slang::FileAccess fileAccess = Slang::FileAccess::Read;
        Slang::FileShare fileShare = Slang::FileShare::None;

        // Open the capture file with read-only access
        SlangResult res = m_inputStream.init(path, fileMode, fileAccess, fileShare);

        if (res != SLANG_OK)
        {
            SlangCapture::slangCaptureLog(SlangCapture::LogLevel::Error, "Failed to open file %s\n", filename.c_str());
            std::abort();
        }
    }

    bool CaptureFileProcessor::processNextBlock()
    {
        FunctionHeader header {};
        if (!processHeader(header))
        {
            return false;
        }

        ApiClassId classId = static_cast<ApiClassId>(getClassId(header.callId));

        // capacity comparison will be performed in the reserve call, so we can safely call reserve
        m_parameterBuffer.reserve(header.dataSizeInBytes);

        size_t readBytes = 0;
        SlangResult res = SLANG_OK;

        if (header.dataSizeInBytes)
        {
            res = m_inputStream.read(m_parameterBuffer.getBuffer(), header.dataSizeInBytes, readBytes);
        }

        if (res != SLANG_OK || readBytes != header.dataSizeInBytes)
        {
            return false;
        }

        FunctionTailer tailer {};
        if (!processTailer(tailer))
        {
            return false;
        }

        if (tailer.dataSizeInBytes)
        {
            m_outputBuffer.reserve(tailer.dataSizeInBytes);
            res = m_inputStream.read(m_outputBuffer.getBuffer(), tailer.dataSizeInBytes, readBytes);

            if (res != SLANG_OK || readBytes != tailer.dataSizeInBytes)
            {
                return false;
            }
        }

        bool ret = false;
        SlangDecoder::ParameterBlock paramBlock {};
        paramBlock.parameterBuffer = m_parameterBuffer.getBuffer();
        paramBlock.parameterBufferSize = header.dataSizeInBytes;
        paramBlock.outputBuffer = m_outputBuffer.getBuffer();
        paramBlock.outputBufferSize = tailer.dataSizeInBytes;

        if (classId == GlobalFunction)
        {
            ret = m_decoder.processFunctionCall(header, paramBlock);
        }
        else
        {
            ret = m_decoder.processMethodCall(header, paramBlock);
        }

        m_parameterBuffer.clear();
        return ret;
    }

    bool CaptureFileProcessor::processHeader(FunctionHeader& header)
    {
        size_t readBytes = 0;
        SlangResult res = m_inputStream.read(&header, sizeof(FunctionHeader), readBytes);

        if (res != SLANG_OK || readBytes != sizeof(FunctionHeader))
        {
            return false;
        }

        if (header.magic != MAGIC_HEADER || header.callId == ApiCallId::InvalidCallId)
        {
            return false;
        }

        return true;
    }

    bool CaptureFileProcessor::processTailer(FunctionTailer& tailer)
    {
        size_t readBytes = 0;
        SlangResult res = m_inputStream.read(&tailer, sizeof(FunctionTailer), readBytes);

        if (res != SLANG_OK || readBytes != sizeof(FunctionTailer))
        {
            return false;
        }

        if (tailer.magic != MAGIC_TAILER)
        {
            return false;
        }

        return true;
    }

    bool CaptureFileProcessor::processMethod(FunctionHeader const& header, const uint8_t* parameterBuffer, int64_t bufferSize)
    {
        return false;
    }

    bool CaptureFileProcessor::processFunction(FunctionHeader const& header, const uint8_t* parameterBuffer, int64_t bufferSize)
    {
        return false;
    }
}; // namespace SlangCapture
