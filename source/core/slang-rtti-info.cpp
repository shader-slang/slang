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

    static RttiInfoManager& getSingleton()
    {
        static RttiInfoManager g_manager;
        return g_manager;
    }

protected:
    RttiInfoManager() :
        m_arena(1024)
    {
    }

    std::recursive_mutex m_mutex;             ///< We need a mutex to guard access to m_arena
    MemoryArena m_arena;
};

/* static */void* RttiInfo::allocate(size_t size)
{
    return RttiInfoManager::getSingleton().allocate(size);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StructRttiBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

StructRttiInfo* StructRttiBuilder::construct()
{
    const Index fieldCount = m_fields.getCount();

    StructRttiInfo* info = (StructRttiInfo*)RttiInfo::allocate(sizeof(StructRttiInfo) + sizeof(StructRttiInfo::Field) * fieldCount);

    info->init(RttiInfo::Kind::Struct, m_alignment, m_sizeInBytes);

    info->m_super = m_super;
    info->m_name = m_name;

    info->m_fieldCount = uint32_t(fieldCount);

    if (fieldCount)
    {
        StructRttiInfo::Field* dstFields = (StructRttiInfo::Field*)(info + 1);
        ::memcpy(dstFields, m_fields.getBuffer(), sizeof(StructRttiInfo::Field) * fieldCount);
    }
    else
    {
        info->m_fields = nullptr;
    }

    return info;
}

} // namespace Slang
