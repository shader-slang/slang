// slang-serialize-type-info.h
#ifndef SLANG_SERIALIZE_TYPE_INFO_H
#define SLANG_SERIALIZE_TYPE_INFO_H

#include "slang-serialize.h"
namespace Slang {

/* For the serialization system to work we need to defined how native types are represented in the serialized format.
This information is defined by specializing SerialTypeInfo with the native type to be converted
This header provides conversion for common Slang types.
*/


// We need to have a way to map between the two.
// If no mapping is needed, (just a copy), then we don't bother with the functions
template <typename T>
struct SerialBasicTypeInfo
{
    typedef T NativeType;
    typedef T SerialType;

    // We want the alignment to be the same as the size of the type for basic types
    // NOTE! Might be different from SLANG_ALIGN_OF(SerialType) 
    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(T*)serial = *(const T*)native; }
    static void toNative(SerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(T*)native = *(const T*)serial; }

    static const SerialType* getType()
    {
        static const SerialType type = { sizeof(SerialType), uint8_t(SerialAlignment), &toSerial, &toNative };
        return &type;
    }
};

template <typename NATIVE_T, typename SERIAL_T>
struct SerialConvertTypeInfo
{
    typedef NATIVE_T NativeType;
    typedef SERIAL_T SerialType;

    enum { SerialAlignment = SerialBasicTypeInfo<SERIAL_T>::SerialAlignment };

    static void toSerial(SerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(SERIAL_T*)serial = SERIAL_T(*(const NATIVE_T*)native); }
    static void toNative(SerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(NATIVE_T*)native = NATIVE_T(*(const SERIAL_T*)serial); }
};

template <typename T>
struct SerialIdentityTypeInfo
{
    typedef T NativeType;
    typedef T SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial) { SLANG_UNUSED(writer); *(T*)serial = *(const T*)native; }
    static void toNative(SerialReader* reader, const void* serial, void* native) { SLANG_UNUSED(reader); *(T*)native = *(const T*)serial; }
};

// Don't need to convert the index type

template <>
struct SerialTypeInfo<SerialIndex> : public SerialIdentityTypeInfo<SerialIndex> {};


// Because is sized, we don't need to convert
template <>
struct SerialTypeInfo<FeedbackType::Kind> : public SerialIdentityTypeInfo<FeedbackType::Kind> {};

// Implement for Basic Types

template <>
struct SerialTypeInfo<uint8_t> : public SerialBasicTypeInfo<uint8_t> {};
template <>
struct SerialTypeInfo<uint16_t> : public SerialBasicTypeInfo<uint16_t> {};
template <>
struct SerialTypeInfo<uint32_t> : public SerialBasicTypeInfo<uint32_t> {};
template <>
struct SerialTypeInfo<uint64_t> : public SerialBasicTypeInfo<uint64_t> {};

template <>
struct SerialTypeInfo<int8_t> : public SerialBasicTypeInfo<int8_t> {};
template <>
struct SerialTypeInfo<int16_t> : public SerialBasicTypeInfo<int16_t> {};
template <>
struct SerialTypeInfo<int32_t> : public SerialBasicTypeInfo<int32_t> {};
template <>
struct SerialTypeInfo<int64_t> : public SerialBasicTypeInfo<int64_t> {};

template <>
struct SerialTypeInfo<float> : public SerialBasicTypeInfo<float> {};
template <>
struct SerialTypeInfo<double> : public SerialBasicTypeInfo<double> {};


// Fixed arrays

template <typename T, size_t N>
struct SerialTypeInfo<T[N]>
{
    typedef SerialTypeInfo<T> ElementASTSerialType;
    typedef typename ElementASTSerialType::SerialType SerialElementType;

    typedef T NativeType[N];
    typedef SerialElementType SerialType[N];

    enum { SerialAlignment = SerialTypeInfo<T>::SerialAlignment };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        SerialElementType* serial = (SerialElementType*)outSerial;
        const T* native = (const T*)inNative;
        for (Index i = 0; i < Index(N); ++i)
        {
            ElementASTSerialType::toSerial(writer, native + i, serial + i);
        }
    }
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        const SerialElementType* serial = (const SerialElementType*)inSerial;
        T* native = (T*)outNative;
        for (Index i = 0; i < Index(N); ++i)
        {
            ElementASTSerialType::toNative(reader, serial + i, native + i);
        }
    }
};

// Special case bool - as we can't rely on size alignment 
template <>
struct SerialTypeInfo<bool>
{
    typedef bool NativeType;
    typedef uint8_t SerialType;

    enum { SerialAlignment = sizeof(SerialType) };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        SLANG_UNUSED(writer);
        *(SerialType*)outSerial = *(const NativeType*)inNative ? 1 : 0;
    }
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        SLANG_UNUSED(reader);
        *(NativeType*)outNative = (*(const SerialType*)inSerial) != 0;
    }
};

// Pointer
// Could handle different pointer base types with some more template magic here, but instead went with Pointer type to keep
// things simpler.
template <typename T>
struct SerialTypeInfo<T*>
{
    typedef T* NativeType;
    typedef SerialIndex SerialType;

    enum
    {
        SerialAlignment = SLANG_ALIGN_OF(SerialType)
    };

    static void toSerial(SerialWriter* writer, const void* inNative, void* outSerial)
    {
        *(SerialType*)outSerial = writer->addPointer(*(T**)inNative);
    }
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        *(T**)outNative = reader->getPointer(*(const SerialType*)inSerial).dynamicCast<T>();
    }
};

// Special case Name
template <>
struct SerialTypeInfo<Name*> : public SerialTypeInfo<RefObject*>
{
    // Special case 
    typedef Name* NativeType;
    static void toNative(SerialReader* reader, const void* inSerial, void* outNative)
    {
        *(Name**)outNative = reader->getName(*(const SerialType*)inSerial);
    }
};

template <>
struct SerialTypeInfo<const Name*> : public SerialTypeInfo<Name*>
{
};

// List
template <typename T, typename ALLOCATOR>
struct SerialTypeInfo<List<T, ALLOCATOR>>
{
    typedef List<T, ALLOCATOR> NativeType;
    typedef SerialIndex SerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        dst = writer->addArray(src.getBuffer(), src.getCount());
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& dst = *(NativeType*)native;
        auto& src = *(const SerialType*)serial;

        reader->getArray(src, dst);
    }
};

// Dictionary
template <typename KEY, typename VALUE>
struct SerialTypeInfo<Dictionary<KEY, VALUE>>
{
    typedef Dictionary<KEY, VALUE> NativeType;
    struct SerialType
    {
        SerialIndex keys;            ///< Index an array
        SerialIndex values;          ///< Index an array
    };

    typedef typename SerialTypeInfo<KEY>::SerialType KeySerialType;
    typedef typename SerialTypeInfo<VALUE>::SerialType ValueSerialType;

    enum { SerialAlignment = SLANG_ALIGN_OF(SerialIndex) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        auto& dst = *(SerialType*)serial;

        List<KeySerialType> keys;
        List<ValueSerialType> values;

        Index count = Index(src.Count());
        keys.setCount(count);
        values.setCount(count);

        Index i = 0;
        for (const auto& pair : src)
        {
            SerialTypeInfo<KEY>::toSerial(writer, &pair.Key, &keys[i]);
            SerialTypeInfo<VALUE>::toSerial(writer, &pair.Value, &values[i]);
            i++;
        }

        dst.keys = writer->addArray(keys.getBuffer(), count);
        dst.values = writer->addArray(values.getBuffer(), count);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;

        // Clear it
        dst = NativeType();

        List<KEY> keys;
        List<VALUE> values;

        reader->getArray(src.keys, keys);
        reader->getArray(src.values, values);

        SLANG_ASSERT(keys.getCount() == values.getCount());

        const Index count = keys.getCount();
        for (Index i = 0; i < count; ++i)
        {
            dst.Add(keys[i], values[i]);
        }
    }
};

// Handle RefPtr - just convert into * to do the conversion
template <typename T>
struct SerialTypeInfo<RefPtr<T>>
{
    typedef RefPtr<T> NativeType;
    typedef typename SerialTypeInfo<T*>::SerialType SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        T* obj = src;
        SerialTypeInfo<T*>::toSerial(writer, &obj, serial);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        T* obj = nullptr;
        SerialTypeInfo<T*>::toNative(reader, serial, &obj);
        *(NativeType*)native = obj;
    }
};

// String
template <>
struct SerialTypeInfo<String>
{
    typedef String NativeType;
    typedef SerialIndex SerialType;
    enum { SerialAlignment = SLANG_ALIGN_OF(SerialType) };

    static void toSerial(SerialWriter* writer, const void* native, void* serial)
    {
        auto& src = *(const NativeType*)native;
        *(SerialType*)serial = writer->addString(src);
    }
    static void toNative(SerialReader* reader, const void* serial, void* native)
    {
        auto& src = *(const SerialType*)serial;
        auto& dst = *(NativeType*)native;
        dst = reader->getString(src);
    }
};

} // namespace Slang

#endif
