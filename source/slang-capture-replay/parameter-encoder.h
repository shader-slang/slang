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
        void encodePointer(ISlangBlob* blob);
        void encodeAddress(const void* value) { encodeValue(reinterpret_cast<uint64_t>(value)); }

        void encodeStruct(slang::SessionDesc const& desc);
        void encodeStruct(slang::PreprocessorMacroDesc const& desc);
        void encodeStruct(slang::CompilerOptionEntry const& entry);
        void encodeStruct(slang::CompilerOptionValue const& value);
        void encodeStruct(slang::TargetDesc const& targetDesc);
        void encodeStruct(slang::SpecializationArg const& specializationArg);

        template <typename T>
        void encodeValueArray(const T* array, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                encodeValue(array[i]);
            }
        }

        void encodeStringArray(const char* const* array, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                encodeString(array[i]);
            }
        }

        template <typename T>
        void encodeStructArray(T const* array, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                encodeStruct(array[i]);
            }
        }

        template <typename T>
        void encodeAddressArray(T* const* array, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                encodeAddress(array[i]);
            }
        }


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
