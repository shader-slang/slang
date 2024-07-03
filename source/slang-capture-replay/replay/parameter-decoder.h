#include <cinttypes>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "../capture-format.h"
#include "../capture-utility.h"
#include "../../../slang.h"

namespace SlangCapture
{
    // This class is used to allocate memory for the type decoder
    class DecoderAllocatorSingleton
    {
    public:
        static DecoderAllocatorSingleton* getInstance()
        {
            thread_local DecoderAllocatorSingleton instance;
            return &instance;
        }

        void* allocate(size_t size)
        {
            void* data = calloc(1, size);
            m_allocations.push_back(data);
            return data;
        }

        ~DecoderAllocatorSingleton()
        {
            for (auto allocation : m_allocations)
            {
                free(allocation);
            }
        }
    private:
        DecoderAllocatorSingleton() = default;
        std::vector<void*> m_allocations;
    };

    class DecoderBase
    {
    public:
        virtual ~DecoderBase() = default;
        void* allocate(size_t size) { return m_allocator->allocate(size); }
    protected:
        DecoderAllocatorSingleton* m_allocator = DecoderAllocatorSingleton::getInstance();
    };

    // We don't allow pointer type to be used as a template parameter
    template <typename T, typename = typename std::enable_if< !std::is_pointer<T>::value >::type>
    class TypeDecoder : public DecoderBase
    {
    public:
        T& getValue() { return m_typeValue;}

    protected:
        T m_typeValue {};
    };

    // We only allow pointer type to be used as a template parameter
    template <class T, typename = typename std::enable_if< std::is_pointer<T>::value >::type>
    class PointerDecoder : public DecoderBase
    {
    public:
        void setPointer(void* data) { m_pointer = static_cast<T>(data); }
        T getPointer()             { return m_pointer; }
        void setPointerAddress(uint64_t address) { m_pointerAddress = address; }
    private:
        T m_pointer {nullptr};
        uint64_t m_pointerAddress = 0;
    };

    class ParameterDecoder
    {
    public:
        static size_t decodeInt8(const uint8_t* buffer,   int64_t bufferSize, int8_t& value)     { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeUint8(const uint8_t* buffer,  int64_t bufferSize, uint8_t& value)   { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeInt16(const uint8_t* buffer,  int64_t bufferSize, int16_t& value)   { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeUint16(const uint8_t* buffer, int64_t bufferSize, uint16_t& value) { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeInt32(const uint8_t* buffer,  int64_t bufferSize, int32_t& value)   { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeUint32(const uint8_t* buffer, int64_t bufferSize, uint32_t& value) { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeInt64(const uint8_t* buffer,  int64_t bufferSize, int64_t& value)   { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeUint64(const uint8_t* buffer, int64_t bufferSize, uint64_t& value) { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeFloat(const uint8_t* buffer,  int64_t bufferSize, float& value)     { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeDouble(const uint8_t* buffer, int64_t bufferSize, double& value)   { return decodeValue(buffer, bufferSize, value); }
        static size_t decodeBool(const uint8_t* buffer,   int64_t bufferSize, bool& value)       { return decodeValue(buffer, bufferSize, value); }

        template<typename T>
        static size_t decodeEnumValue(const uint8_t* buffer, size_t bufferSize, T& value)
        {
            uint32_t decodedValue;
            size_t readByte = decodeValue(buffer, bufferSize, decodedValue);
            value = static_cast<T>(decodedValue);
            return readByte;
        }

        static size_t decodeString(const uint8_t* buffer,  int64_t bufferSize, PointerDecoder<char*>& typeDecoder);

        static size_t decodePointer(const uint8_t* buffer, int64_t bufferSize, PointerDecoder<void*>& pointerDecoder);
        static void decodePointer(ISlangBlob* blob);

        static size_t decodeAddress(const uint8_t* buffer, int64_t bufferSize, SlangCapture::AddressFormat& address)
        {
            return decodeValue(buffer, bufferSize, address);
        }
        static size_t decodeStruct(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<slang::SessionDesc>& sessionDesc);
        static size_t decodeStruct(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<slang::PreprocessorMacroDesc>& desc);
        static size_t decodeStruct(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<slang::CompilerOptionEntry>& entry);
        static size_t decodeStruct(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<slang::CompilerOptionValue>& value);
        static size_t decodeStruct(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<slang::TargetDesc>& targetDesc);
        static size_t decodeStruct(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<slang::SpecializationArg>& specializationArg);

        template <typename T>
        static size_t decodeValueArray(const uint8_t* buffer, int64_t bufferSize, TypeDecoder<T>* valueArray, size_t count)
        {
            SLANG_CAPTURE_ASSERT((buffer != nullptr) && (bufferSize > 0));

            size_t readByte = 0;
            for (size_t i = 0; i < count; ++i)
            {
                readByte += decodeValue(buffer + readByte, bufferSize - readByte, valueArray[i]);
            }
            return readByte;
        }

        static size_t decodeStringArray(const uint8_t* buffer, int64_t bufferSize, char** outputArray, size_t count)
        {
            SLANG_CAPTURE_ASSERT((buffer != nullptr) && (bufferSize > 0));

            size_t readByte = 0;
            for (uint32_t i = 0; i < count; i++)
            {
                PointerDecoder<char*> item;
                readByte += decodeString(buffer + readByte, bufferSize - readByte, item);

                // Copy the search path
                outputArray[i] = item.getPointer();
            }
            return readByte;
        }

        template <typename T>
        static size_t decodeStructArray(const uint8_t* buffer, int64_t bufferSize, T* outputArray, size_t count)
        {
            SLANG_CAPTURE_ASSERT((buffer != nullptr) && (bufferSize > 0));

            size_t bufferRead = 0;
            for (size_t i = 0; i < count; ++i)
            {
                TypeDecoder<T> item;
                bufferRead += decodeStruct(buffer + bufferRead, bufferSize - bufferRead, item);
                outputArray[i] = item.getValue();
            }
            return bufferRead;
        }

        template <typename T>
        static size_t encodeAddressArray(const uint8_t* buffer, int64_t bufferSize, uint64_t* addressArray, size_t count)
        {
            SLANG_CAPTURE_ASSERT((buffer != nullptr) && (bufferSize > 0));

            size_t bufferRead = 0;
            for (size_t i = 0; i < count; ++i)
            {
                bufferRead += decodeAddress(buffer + bufferRead, bufferSize - bufferRead, addressArray[i]);
            }
        }


    private:
        template <typename T>
        static size_t decodeValue(const uint8_t* buffer, int64_t bufferSize, T& value)
        {
            SLANG_CAPTURE_ASSERT((buffer != nullptr) && (bufferSize > 0));

            int64_t dataSize  = sizeof(T);

            SLANG_CAPTURE_ASSERT(bufferSize >= dataSize);

            size_t bytesRead = 0;
            bytesRead = dataSize;
            memcpy(&value, buffer, dataSize);

            return bytesRead;
        }
    };
} // namespace SlangCapture
