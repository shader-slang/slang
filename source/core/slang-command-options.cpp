// slang-command-options.cpp

#include "slang-command-options.h"

#include "slang-string-util.h"

namespace Slang {
   
Index CommandOptions::_addOptionName(const UnownedStringSlice& name, Flags flags)
{
    StringSlicePool::Handle handle;
    if (m_optionPool.findOrAdd(name, handle))
    {
        SLANG_ASSERT(!"Option is already added!");
        return -1;
    }

    // Add to prefix 
    if (flags & (Flag::CanPrefix | Flag::IsPrefix))
    {
        const auto length = name.getLength();
        SLANG_ASSERT(length < 32);
        m_prefixSizes |= uint32_t(1) << length;
    }

    return Index(handle);
}

UnownedStringSlice CommandOptions::_addString(const char* text)
{
    if (text == nullptr)
    {
        return UnownedStringSlice();
    }
    return _addString(UnownedStringSlice(text));
}

UnownedStringSlice CommandOptions::_addString(const UnownedStringSlice& slice)
{
    const auto length = slice.getLength();
    const char* dst = m_arena.allocateString(slice.begin(), length);
    return UnownedStringSlice(dst, length);
}


Index CommandOptions::_addOption(const UnownedStringSlice* names, Count namesCount, const Option& inOption)
{
    SLANG_ASSERT(namesCount > 0);

    const auto startIndex = _addOptionName(names[0], inOption.flags);

    for (Index i = 1; i < namesCount; ++i)
    {
        const auto curIndex = _addOptionName(names[i], inOption.flags);
        // Make sure it's what we expect
        SLANG_ASSERT(startIndex + i == curIndex);
    }

    const Index endIndex = startIndex + namesCount;
    const Index optionIndex = m_options.getCount();

    {
        const Count oldCount = m_optionMap.getCount();

        m_optionMap.setCount(endIndex);
        auto dst = m_optionMap.getBuffer();

        for (Index i = oldCount; i < startIndex; ++i)
        {
            dst[i] = -1;
        }
        for (Index i = startIndex; i < endIndex; ++i)
        {
            dst[i] = optionIndex;
        }
    }

    Option option(inOption);

    option.startNameIndex = startIndex;
    option.endNameIndex = endIndex;

    // Set the first name as the "listed" name
    option.name = m_optionPool.getSlice(StringSlicePool::Handle(startIndex));

    m_options.add(option);
    return optionIndex;
}

void CommandOptions::add(const char* name, const char* usage, const char* description, Flags flags)
{
    const UnownedStringSlice nameSlice(name);

    Option option;
    option.categoryIndex = m_currentCategoryIndex;
    option.usage = _addString(usage);
    option.description = _addString(UnownedStringSlice(description));
    option.flags = flags;

    if (nameSlice.indexOf(',') >= 0)
    {
        List<UnownedStringSlice> names;
        StringUtil::split(nameSlice, ',', names);

        _addOption(names.getBuffer(), names.getCount(), option);
    }
    else
    {
        _addOption(&nameSlice, 1, option);
    }
}

void CommandOptions::add(const UnownedStringSlice* names, Count namesCount, const char* usage, const char* description, Flags flags)
{
    Option option;
    option.categoryIndex = m_currentCategoryIndex;
    option.usage = _addString(usage);
    option.description = _addString(UnownedStringSlice(description));
    option.flags = flags;

    _addOption(names, namesCount, option);
}

Index CommandOptions::addCategory(const char* name, const char* description)
{
    const UnownedStringSlice nameSlice(name);

    for (auto& category : m_categories)
    {
        if (category.name == nameSlice)
        {
            SLANG_ASSERT(!"Already has category");
            return -1;
        }
    }

    Category cat;
    cat.name = _addString(nameSlice);
    cat.description = _addString(description);

    m_categories.add(cat);

    return m_categories.getCount() - 1;
}

void CommandOptions::setCategory(const char* name)
{
    const UnownedStringSlice nameSlice(name);

    for (Index i = 0; i < m_categories.getCount(); ++i)
    {
        auto& cat = m_categories[i];
        if (cat.name == nameSlice)
        {
            m_currentCategoryIndex = i;
            return;
        }
    }

    SLANG_ASSERT(!"Category not found");

    m_currentCategoryIndex = -1;
}

void CommandOptions::appendDescription(StringBuilder& buf)
{
    // Go through categories in order

    const auto indentSlice = toSlice("  ");

    for (Index categoryIndex = 0; categoryIndex < m_categories.getCount(); ++categoryIndex)
    {
        const auto& category = m_categories[categoryIndex];
        
        buf << category.description << "\n";
        buf.appendRepeatedChar('=', category.description.getLength());
        buf << "\n\n";

        for (auto& option : m_options)
        {
            if (option.categoryIndex != categoryIndex)
            {
                continue;
            }

            buf << indentSlice;

            // If we have usage just output that
            if (option.usage.getLength())
            {
                buf << option.usage;
            }
            else
            {
                List<UnownedStringSlice> names;
                for (Index i = option.startNameIndex; i < option.endNameIndex; ++i)
                {
                    names.add(m_optionPool.getSlice(StringSlicePool::Handle(i)));
                }

                StringUtil::join(names.getBuffer(), names.getCount(), toSlice(", "), buf);
            }

            buf << toSlice(": ");

            List<UnownedStringSlice> lines;
            StringUtil::calcLines(option.description, lines);
                
            // Remove very last line if it's empty
            if (lines.getCount() > 1 && lines.getLast().trim().getLength() == 0)
            {
                lines.removeLast();
            }

            buf << lines[0] << "\n";

            for (Index i = 1; i < lines.getCount(); ++i)
            {
                buf << indentSlice << indentSlice << lines[i] << "\n";
            }

            //buf << "\n";
        }

        buf << "\n";
    }
}

} // namespace Slang


