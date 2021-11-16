#ifndef SLANG_CORE_RTTI_INFO_H
#define SLANG_CORE_RTTI_INFO_H

#include "slang-basic.h"
#include "slang-memory-arena.h"

#include "slang-list.h"
#include "slang-dictionary.h"

namespace Slang {

struct RttiInfo
{
    typedef uint8_t AlignmentType;
    typedef uint16_t SizeType;

    enum class Kind : uint8_t
    {
        Invalid,
        I32,
        U32,
        I64,
        U64,
        F32,
        F64,
        Bool,
        String,
        UnownedStringSlice,
        Struct,
        Enum,
        List,
        Dictionary,
        Other,
        CountOf,
    };

    Kind m_kind;
    AlignmentType m_alignment;
    SizeType m_size;

    void init(Kind kind, size_t alignment, size_t size) { m_kind = kind; m_alignment = AlignmentType(alignment); m_size = SizeType(size); }

    template <typename T>
    void init(Kind kind) { init(kind, SLANG_ALIGN_OF(T), sizeof(T)); }
    
        /// Allocate memory for RttiInfo types.
        /// Is thread safe, and doesn't require the memory to be freed explicitly
        /// Will be freed at shutdown (via global dtor)
    static void* allocate(size_t size);

    static const RttiInfo g_basicTypes[Index(Kind::CountOf)];
};

struct StructRttiInfo : public RttiInfo
{
    typedef uint8_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            Optional = 0x1,
        };
    };

    struct Field
    {
        const char* m_name;             ///< Name of this field
        const RttiInfo* m_type;         ///< The type of this field
        uint32_t m_offset;              ///< Offset from object type in bytes
        Flags m_flags;                  ///< Field flags
    };

    const char* m_name;                 ///< Name
    const StructRttiInfo* m_super;      ///< Super class or nullptr if not defined

    Index m_fieldCount;                 ///< Amount of fields
    const Field* m_fields;              ///< Fields
};

struct ListRttiInfo : public RttiInfo
{
    RttiInfo* m_elementType;
};

struct DictionaryRttiInfo : public RttiInfo
{
    RttiInfo* m_keyType;
    RttiInfo* m_valueType;
};

template <typename T>
struct GetRttiInfo;

template <> struct GetRttiInfo<bool> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::Bool)];} };
template <> struct GetRttiInfo<int32_t> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::I32)]; } };
template <> struct GetRttiInfo<int64_t> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::I64)]; } };
template <> struct GetRttiInfo<uint32_t> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::U32)]; } };
template <> struct GetRttiInfo<uint64_t> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::U64)]; } };
template <> struct GetRttiInfo<float> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::F32)]; } };
template <> struct GetRttiInfo<double> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::F64)]; } };
template <> struct GetRttiInfo<String> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::String)]; } };
template <> struct GetRttiInfo<UnownedStringSlice> { static const RttiInfo* get() { return &RttiInfo::g_basicTypes[Index(RttiInfo::Kind::UnownedStringSlice)]; } };

template <typename T>
struct GetRttiInfo<List<T>>
{
    static const RttiInfo* _create()
    {
        auto info = (ListRttiInfo*)RttiInfo::allocate(sizeof(ListRttiInfo));
        info->init<List<Byte>>(RttiInfo::Kind::List);
        info->m_elementType = GetRttiInfo<T>::get();
        return info;
    }
    static const RttiInfo* get() { static const RttiInfo* g_info = _create(); return g_info; }
};

// Strip const
template <typename T>
struct GetRttiInfo<const T>
{
    static const RttiInfo* get() { return GetRttiInfo<T>::get(); }
};

template <typename K, typename V>
struct GetRttiInfo<Dictionary<K, V>>
{
    static const RttiInfo* _create()
    {
        auto info = (DictionaryRttiInfo*)allocate(sizeof(DictionaryRttiInfo)); 
        info->init<Dictionary<Byte, Byte>>(RttiInfo::Kind::Dictionary);
        info->m_keyType = GetRttiInfo<K>::get();
        info->m_valueType = GetRttiInfo<V>::get();
        return info;
    }
    static const RttiInfo* get() { static const RttiInfo* g_info = _create(); return g_info; }
};

struct StructRttiBuilder
{
    template <typename T>
    StructRttiBuilder(T* obj, const char* name, const StructRttiInfo* super) :
        m_base((const Byte*)obj),
        m_super(super),
        m_sizeInBytes(uint32_t(sizeof(T))),
        m_name(name)
    {
        m_alignment = uint8_t(SLANG_ALIGN_OF(T));
    }

    template <typename T>
    void addField(const char* name, const T* fieldPtr, StructRttiInfo::Flags flags = 0)
    {
        StructRttiInfo::Field field;

        field.m_name = name;
        field.m_type = GetRttiInfo<T>::get();
        field.m_offset = uint32_t(ptrdiff_t((const Byte*)fieldPtr - m_base));
        field.m_flags = flags;
        m_fields.add(field);
    }

    StructRttiInfo* construct();

    const char* m_name; 
    const StructRttiInfo* m_super = nullptr;
    List<StructRttiInfo::Field> m_fields;
    const Byte* m_base;
    uint32_t m_sizeInBytes;
    uint8_t m_alignment;
};

#define SLANG_STRUCT_RTTI_INFO(x) \
template <> \
struct GetRttiInfo<x> \
{ \
    static const RttiInfo* get() \
    { \
        const RttiInfo* g_info = x::createRttiInfo(); \
        return g_info; \
    } \
};

} // namespace Slang

#endif // SLANG_CORE_RTTI_INFO_H
