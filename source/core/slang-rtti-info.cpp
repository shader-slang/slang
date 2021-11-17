#include "slang-rtti-info.h"

#include "../../slang-com-helper.h"

#include <mutex>

namespace Slang {

#define SLANG_RTTI_INFO_INVALID(name) RttiInfo{RttiInfo::Kind::Invalid, 0, 0}
#define SLANG_RTTI_INFO_BASIC(name, type) \
    RttiInfo{RttiInfo::Kind::name, RttiInfo::AlignmentType(SLANG_ALIGN_OF(type)), RttiInfo::SizeType(sizeof(type))}

/* static */const RttiInfo RttiInfo::g_basicTypes[Index(Kind::CountOf)] =
{
    SLANG_RTTI_INFO_INVALID(Invalid),
    SLANG_RTTI_INFO_BASIC(I32, int32_t),
    SLANG_RTTI_INFO_BASIC(U32, uint32_t),
    SLANG_RTTI_INFO_BASIC(I64, int64_t),
    SLANG_RTTI_INFO_BASIC(U64, uint64_t),
    SLANG_RTTI_INFO_BASIC(F32, float),
    SLANG_RTTI_INFO_BASIC(F64, double),
    SLANG_RTTI_INFO_BASIC(Bool, bool),
    SLANG_RTTI_INFO_BASIC(String, String),
    SLANG_RTTI_INFO_BASIC(UnownedStringSlice, UnownedStringSlice),
    SLANG_RTTI_INFO_BASIC(Ptr, void*),
    SLANG_RTTI_INFO_BASIC(RefPtr, RefPtr<StringRepresentation>),
    SLANG_RTTI_INFO_INVALID(FixedArray),
    SLANG_RTTI_INFO_INVALID(Struct),
    SLANG_RTTI_INFO_INVALID(Enum),
    SLANG_RTTI_INFO_INVALID(List),
    SLANG_RTTI_INFO_INVALID(Dictionary),
    SLANG_RTTI_INFO_INVALID(Other),
};

struct RttiInfoManager
{
    void* allocate(size_t size)
    {
        std::lock_guard<std::recursive_mutex> guard(m_mutex);
        return m_arena.allocate(size);
    }

    void addDynamicArrayFuncs(size_t alignment, size_t size, const RttiDynamicArrayFuncs& funcs)
    {
        SLANG_UNUSED(alignment);
        // 1, 2, 4, 8

        switch (size)
        {
            case 1:
            case 2:
            case 4:
            case 8:
            {
                return;
            }
        }

        {
            std::lock_guard<std::recursive_mutex> guard(m_mutex);
            m_dynamicArrayFuncsMap.TryGetValueOrAdd(size, funcs);
        }
    }

    RttiDynamicArrayFuncs getDynamicArrayFuncs(size_t alignment, size_t size)
    {
        SLANG_UNUSED(alignment);
        switch (size)
        {
            case 1:     return m_builtInFuncs[0];
            case 2:     return m_builtInFuncs[1];
            case 4:     return m_builtInFuncs[2];
            case 8:     return m_builtInFuncs[3];
        }

        {
            std::lock_guard<std::recursive_mutex> guard(m_mutex);

            auto ptr = m_dynamicArrayFuncsMap.TryGetValue(size);
            if (ptr)
            {
                return *ptr;
            }
            RttiDynamicArrayFuncs empty{};
            return empty;
        }
    }

    template <typename T>
    void addDynamicArrayFuncs()
    {
        addDynamicArrayFuncs(SLANG_ALIGN_OF(T), sizeof(T), getDynamicArrayFuncsForType<T>());
    }

    static RttiInfoManager& getSingleton()
    {
        static RttiInfoManager g_manager;
        return g_manager;
    }

protected:
    RttiInfoManager() :
        m_arena(1024)
    {
        m_builtInFuncs[0] = getDynamicArrayFuncsForType<uint8_t>();
        m_builtInFuncs[1] = getDynamicArrayFuncsForType<uint16_t>();
        m_builtInFuncs[2] = getDynamicArrayFuncsForType<uint32_t>();
        m_builtInFuncs[3] = getDynamicArrayFuncsForType<uint64_t>();

        addDynamicArrayFuncs<String>();
        addDynamicArrayFuncs<UnownedStringSlice>();
        addDynamicArrayFuncs<List<Byte>>();
        addDynamicArrayFuncs<Dictionary<Byte, Byte>>();
    }

    RttiDynamicArrayFuncs m_builtInFuncs[4];
    Dictionary<size_t, RttiDynamicArrayFuncs> m_dynamicArrayFuncsMap;

    std::recursive_mutex m_mutex;             ///< We need a mutex to guard access to m_arena
    MemoryArena m_arena;
};

/* static */void* RttiInfo::allocate(size_t size)
{
    return RttiInfoManager::getSingleton().allocate(size);
}

/* static */void RttiInfo::addDynamicArrayFuncs(size_t alignment, size_t size, const RttiDynamicArrayFuncs& funcs)
{
    RttiInfoManager::getSingleton().addDynamicArrayFuncs(alignment, size, funcs);
}

/* static */RttiDynamicArrayFuncs RttiInfo::getDynamicArrayFuncs(size_t alignment, size_t size)
{
    return RttiInfoManager::getSingleton().getDynamicArrayFuncs(alignment, size);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StructRttiBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void StructRttiBuilder::_init(const char* name, const StructRttiInfo* super, const Byte* base)
{
    m_rttiInfo.m_name = name;
    m_rttiInfo.m_super = super;
    m_base = base;

    m_rttiInfo.m_fieldCount = 0;
    m_rttiInfo.m_fields = nullptr;
}

StructRttiInfo StructRttiBuilder::make()
{
    const Index fieldCount = m_fields.getCount();

    if (fieldCount)
    {
        StructRttiInfo::Field* dstFields = (StructRttiInfo::Field*)RttiInfo::allocate(sizeof(StructRttiInfo::Field) * fieldCount);
        ::memcpy(dstFields, m_fields.getBuffer(), sizeof(StructRttiInfo::Field) * fieldCount);

        m_rttiInfo.m_fields = dstFields;
        m_rttiInfo.m_fieldCount = fieldCount;
    }

    return m_rttiInfo;
}

} // namespace Slang
