#include "identifier-lookup.h"

namespace CppExtract {
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

void IdentifierLookup::initDefault(const UnownedStringSlice& markPrefix)
{
    reset();

    // Some keywords
    {
        const char* names[] = { "virtual", "typedef", "continue", "if", "case", "break", "catch", "default", "delete", "do", "else", "for", "new", "goto", "return", "switch", "throw", "using", "while", "operator" };
        set(names, SLANG_COUNT_OF(names), IdentifierStyle::Keyword);
    }

    // Type modifier keywords
    {
        const char* names[] = { "const", "volatile" };
        set(names, SLANG_COUNT_OF(names), IdentifierStyle::TypeModifier);
    }

    // Special markers
    {
        const char* names[] = { "PRE_DECLARE", "TYPE_SET", "REFLECTED", "UNREFLECTED" };
        const IdentifierStyle styles[] = { IdentifierStyle::PreDeclare, IdentifierStyle::TypeSet, IdentifierStyle::Reflected, IdentifierStyle::Unreflected };
        SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(names) == SLANG_COUNT_OF(styles));

        StringBuilder buf;
        for (Index i = 0; i < SLANG_COUNT_OF(names); ++i)
        {
            buf.Clear();
            buf << markPrefix << names[i];
            set(buf.getUnownedSlice(), styles[i]);
        }
    }

    // Keywords which introduce types/scopes
    {
        set("struct", IdentifierStyle::Struct);
        set("class", IdentifierStyle::Class);
        set("namespace", IdentifierStyle::Namespace);
    }

    // Keywords that control access
    {
        const char* names[] = { "private", "protected", "public" };
        set(names, SLANG_COUNT_OF(names), IdentifierStyle::Access);
    }
}

} // namespace CppExtract
