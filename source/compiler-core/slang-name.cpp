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
    RefPtr<Name> name;
    if (names.tryGetValue(text, name))
        return name;

    name = new Name();
    name->text = text;
    // Key the map on the `Name`'s own string rather than on `text` again. `String` is
    // reference-counted, so this shares one buffer between the map and the `Name` instead of
    // copying the characters a second time.
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
