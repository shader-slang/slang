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
        void EncodeInt8(int8_t value) { EncodeValue(value); }
        void EncodeUint8(uint8_t value) { EncodeValue(value); }
        void EncodeInt16(int16_t value) { EncodeValue(value); }
        void EncodeUint16(uint16_t value) { EncodeValue(value); }
        void EncodeInt32(int32_t value) { EncodeValue(value); }
        void EncodeUint32(uint32_t value) { EncodeValue(value); }
        void EncodeInt64(int64_t value) { EncodeValue(value); }
        void EncodeUint64(uint64_t value) { EncodeValue(value); }
        void EncodeFloat(float value) { EncodeValue(value); }
        void EncodeDouble(double value) { EncodeValue(value); }
        void EncodeBool(bool value) { EncodeValue(value); }

        template<typename T>
        void EncodeEnumValue(T value) { EncodeValue(static_cast<uint32_t>(value)); }

        void EncodeString(const char* value);
        void EncodePointer(const void* value);
    private:
        template <typename T>
        void EncodeValue(T value)
        {
            m_stream->Write(&value, sizeof(T));
        }
        OutputStream* m_stream;
    };
} // namespace SlangCapture

#endif // PARAMETER_ENCODER_H
