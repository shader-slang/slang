#ifndef PARAMETER_ENCODER_H
#define PARAMETER_ENCODER_H

#include <cstdio>
#include <cinttypes>
#include <cstdint>

#include "output-stream.h"

namespace SlangCapture
{
    class ParameterEncoder
    {
    public:
        ParameterEncoder(OutputStream* stream) : m_stream(stream) {};
        void encodeInt8(int8_t value) { encodeValue(value); }
        void encodeUint8(uint8_t value) { encodeValue(value); }
        void encodeInt16(int16_t value) { encodeValue(value); }
        void encodeUint16(uint16_t value) { encodeValue(value); }
        void encodeInt32(int32_t value) { encodeValue(value); }
        void encodeUint32(uint32_t value) { encodeValue(value); }
        void encodeInt64(int64_t value) { encodeValue(value); }
        void encodeUint64(uint64_t value) { encodeValue(value); }
        void encodeFloat(float value) { encodeValue(value); }
        void encodeDouble(double value) { encodeValue(value); }
        void encodeBool(bool value) { encodeValue(value); }

        template<typename T>
        void encodeEnumValue(T value) { encodeValue(static_cast<uint32_t>(value)); }

        void encodeString(const char* value);
        void encodePointer(const void* value, bool omitData = false, size_t size = 0);
        void encodeAddress(const void* value) { encodeValue(reinterpret_cast<uint64_t>(value)); }

        void encodeStruct(slang::SessionDesc const& desc);
        void encodeStruct(slang::PreprocessorMacroDesc const& desc);
        void encodeStruct(slang::CompilerOptionEntry const& entry);
        void encodeStruct(slang::CompilerOptionValue const& value);
        void encodeStruct(slang::TargetDesc const& targetDesc);

    private:
        template <typename T>
        void encodeValue(T value)
        {
            m_stream->write(&value, sizeof(T));
        }
        OutputStream* m_stream;
    };
} // namespace SlangCapture

#endif // PARAMETER_ENCODER_H
