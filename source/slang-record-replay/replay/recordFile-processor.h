#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <string>
#include "../../core/slang-stream.h"
#include "../util/record-format.h"
#include "slang-decoder.h"

namespace SlangRecord
{
    class RecordFileProcessor
    {
    public:
        RecordFileProcessor(const std::string& filePath);
        bool processNextBlock();
        bool processHeader(FunctionHeader& header);
        bool processTailer(FunctionTailer& tailer);
        bool processMethod(FunctionHeader const& header, const uint8_t* buffer, int64_t bufferSize);
        bool processFunction(FunctionHeader const& header, const uint8_t* buffer, int64_t bufferSize);
    private:
        Slang::FileStream       m_inputStream;
        Slang::List<uint8_t>    m_parameterBuffer;
        Slang::List<uint8_t>    m_outputBuffer;

        SlangDecoder            m_decoder;
    };

} // namespace SlangRecord;

#endif // FILE_PROCESSOR_H
