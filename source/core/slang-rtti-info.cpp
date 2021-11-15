#include "slang-rtti-info.h"

#include "../../slang-com-helper.h"

namespace Slang {

/* static */const RttiInfo RttiInfo::g_basicTypes[Index(Kind::CountOf)] =
{
    RttiInfo{RttiInfo::Kind::Invalid},
    RttiInfo{RttiInfo::Kind::I32},
    RttiInfo{RttiInfo::Kind::U32},
    RttiInfo{RttiInfo::Kind::I64},
    RttiInfo{RttiInfo::Kind::U64},
    RttiInfo{RttiInfo::Kind::F32},
    RttiInfo{RttiInfo::Kind::F64},
    RttiInfo{RttiInfo::Kind::Bool},
    RttiInfo{RttiInfo::Kind::String},
    RttiInfo{RttiInfo::Kind::UnownedStringSlice},
    RttiInfo{RttiInfo::Kind::Invalid},  /// Struct,
    RttiInfo{RttiInfo::Kind::Invalid},  /// Enum
    RttiInfo{RttiInfo::Kind::Invalid},  /// List
    RttiInfo{RttiInfo::Kind::Invalid},  /// Dictionary
    RttiInfo{RttiInfo::Kind::Invalid},  /// Other
};

/* static */MemoryArena& RttiInfo::getMemoryArena()
{
    static MemoryArena g_arena(1024);
    return g_arena;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StructRttiBuilder !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

StructRttiInfo* StructRttiBuilder::construct()
{
    auto& arena = RttiInfo::getMemoryArena();

    const Index fieldCount = m_fields.getCount();

    StructRttiInfo* info = (StructRttiInfo*)arena.allocate(sizeof(StructRttiInfo) + sizeof(StructRttiInfo::Field) * fieldCount);
    info->m_kind = RttiInfo::Kind::Struct;
    info->m_sizeInBytes = m_sizeInBytes;
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
