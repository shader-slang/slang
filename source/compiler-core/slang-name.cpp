// slang-name.cpp
#include "slang-name.h"

namespace Slang
{

String getText(Name* name)
{
    if (!name)
        return String();
    return name->text;
}

UnownedStringSlice getUnownedStringSliceText(Name* name)
{
    return name ? name->text.getUnownedSlice() : UnownedStringSlice();
}

const char* getCstr(Name* name)
{
    return name ? name->text.getBuffer() : nullptr;
}

Name* NamePool::getName(UnownedStringSlice text)
{
    // This runs for every identifier the lexer produces, so the hit path is
    // kept free of allocation: `text` is used to probe the map directly rather
    // than being converted to a `String` first.
    if (const auto name = names.tryGetValue(text))
        return *name;

    RefPtr<Name> name = new Name();
    name->text = text;
    // Key the entry on the characters the `Name` itself now owns. Copying a
    // `String` shares its representation, so the pool ends up holding one copy
    // of the text rather than two.
    names.add(name->text, name);
    return name;
}

Name* NamePool::getName(String const& text)
{
    return getName(text.getUnownedSlice());
}

Name* NamePool::tryGetName(String const& text)
{
    RefPtr<Name> name;
    if (names.tryGetValue(text, name))
        return name;
    return nullptr;
}

} // namespace Slang
