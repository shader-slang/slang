#include "identifier-lookup.h"

namespace SlangExperimental {
using namespace Slang;

/* static */const IdentifierFlags IdentifierLookup::kIdentifierFlags[Index(IdentifierStyle::CountOf)] =
{
    0,              /// None
    0,              /// Identifier
    0,              /// Declare type
    0,              /// Type set
    IdentifierFlag::Keyword,              /// TypeModifier
    IdentifierFlag::Keyword,              /// Keyword
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::ClassLike, /// Class
    IdentifierFlag::Keyword | IdentifierFlag::StartScope | IdentifierFlag::ClassLike, /// Struct
    IdentifierFlag::Keyword | IdentifierFlag::StartScope, /// Namespace
    IdentifierFlag::Keyword,                              /// Access
    IdentifierFlag::Reflection,                           /// Reflected
    IdentifierFlag::Reflection,                           /// Unreflected
};


void IdentifierLookup::set(const UnownedStringSlice& name, IdentifierStyle style)
{
    StringSlicePool::Handle handle;
    if (m_pool.findOrAdd(name, handle))
    {
        // Add the extra flags
        m_styles[Index(handle)] = style;
    }
    else
    {
        Index index = Index(handle);
        SLANG_ASSERT(index == m_styles.getCount());
        m_styles.add(style);
    }
}

void IdentifierLookup::set(const char*const* names, size_t namesCount, IdentifierStyle style)
{
    for (size_t i = 0; i < namesCount; ++i)
    {
        set(UnownedStringSlice(names[i]), style);
    }
}


} // namespace SlangExperimental
