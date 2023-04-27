// slang-command-options.cpp

#include "slang-command-options.h"

#include "slang-string-util.h"
#include "slang-char-util.h"

namespace Slang {
   
SlangResult CommandOptions::_addName(LookupKind kind, const UnownedStringSlice& name, Index targetIndex)
{
    NameKey nameKey;
    nameKey.kind = kind;
    nameKey.nameIndex = (Index)m_pool.add(name);

    if (m_nameMap.tryGetValueOrAdd(nameKey, targetIndex))
    {
        SLANG_ASSERT(!"Option is already added!");
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

SlangResult CommandOptions::_addOptionName(const UnownedStringSlice& name, Flags flags, Index targetIndex)
{
    SLANG_RETURN_ON_FAIL(_addName(LookupKind::Option, name, targetIndex));

    // Add to prefix flags
    if (flags & (Flag::CanPrefix | Flag::IsPrefix))
    {
        const auto length = name.getLength();
        SLANG_ASSERT(length < 32);
        m_prefixSizes |= uint32_t(1) << length;
    }

    return SLANG_OK;
}

SlangResult CommandOptions::_addValueName(const UnownedStringSlice& name, Index categoryIndex, Index optionIndex)
{
    return _addName(LookupKind(categoryIndex), name, optionIndex);
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

Index CommandOptions::_addOption(const UnownedStringSlice& name, const Option& inOption)
{
    return _addOption(&name, 1, inOption);
}

Index CommandOptions::_addOption(const UnownedStringSlice* names, Count namesCount, const Option& inOption)
{
    SLANG_ASSERT(namesCount > 0);
    SLANG_ASSERT(inOption.categoryIndex >= 0);

    if (namesCount <= 0 || inOption.categoryIndex < 0)
    {
        return -1;
    }

    auto& cat = m_categories[inOption.categoryIndex];

    // If there are already options associated with this category, we have to be in the run of the last ones added
    if (cat.optionStartIndex != cat.optionEndIndex)
    {
        // If we aren't at the end then this is an error
        if (cat.optionEndIndex != m_options.getCount())
        {
            return -1;
        }
    }
    else
    {
        // Move to the end of the option list
        cat.optionStartIndex = m_options.getCount();
        cat.optionEndIndex = cat.optionStartIndex;
    }

    Option option(inOption);

    const Index optionIndex = m_options.getCount();

    if (cat.kind == CategoryKind::Option)
    {
        for (Index i = 0; i < namesCount; ++i)
        {
            _addOptionName(names[i], inOption.flags, optionIndex);
        }
    }
    else
    {
        for (Index i = 0; i < namesCount; ++i)
        {
            _addValueName(names[i], inOption.categoryIndex, optionIndex); 
        }
    }

    if (namesCount == 1 && cat.kind == CategoryKind::Option)
    {
        // We already have storage on the slice
        option.names = m_pool.addAndGetSlice(names[0]);
    }
    else
    {
        // Put all of the names in the list
        StringBuilder buf;
        StringUtil::join(names, namesCount, ',', buf);
        // Allocate storage no in the pool
        option.names = _addString(buf.getUnownedSlice());
    }

    m_options.add(option);

    // Set the end index
    cat.optionEndIndex = m_options.getCount();

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

Index CommandOptions::_addValue(const UnownedStringSlice& name, const Option& inOption)
{
    SLANG_ASSERT(m_currentCategoryIndex >= 0);
    SLANG_ASSERT(m_categories[m_currentCategoryIndex].kind == CategoryKind::Value);

    return _addOption(name, inOption);
}

void CommandOptions::addValue(const UnownedStringSlice& name)
{
    Option option;
    option.categoryIndex = m_currentCategoryIndex;
    _addValue(name, option);
}

void CommandOptions::addValue(const UnownedStringSlice& name, const UnownedStringSlice& description)
{
    Option option;
    option.categoryIndex = m_currentCategoryIndex;
    option.description = _addString(description);
    _addValue(name, option);
}

void CommandOptions::addValue(const UnownedStringSlice* names, Count namesCount)
{
    Option option;
    option.categoryIndex = m_currentCategoryIndex;

    SLANG_ASSERT(m_currentCategoryIndex >= 0);
    SLANG_ASSERT(m_categories[m_currentCategoryIndex].kind == CategoryKind::Value);

    _addOption(names, namesCount, option);
}

void CommandOptions::addValue(const char* name, const char* description)
{
    addValue(UnownedStringSlice(name), UnownedStringSlice(description));
}

void CommandOptions::addValue(const char* name)
{
    addValue(UnownedStringSlice(name));
}

Index CommandOptions::addCategory(CategoryKind kind, const char* name, const char* description)
{
    const UnownedStringSlice nameSlice(name);

    const auto categoryIndex = m_categories.getCount();

    if (SLANG_FAILED(_addName(LookupKind::Category, nameSlice, categoryIndex)))
    {
        return -1;
    }

    Category cat;
    cat.kind = kind;
    cat.name = _addString(nameSlice);
    cat.description = _addString(description);

    m_currentCategoryIndex = categoryIndex;

    m_categories.add(cat);

    return categoryIndex;
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

Index CommandOptions::findTargetIndexByName(LookupKind kind, const UnownedStringSlice& name) const
{
    const auto nameIndex = m_pool.findIndex(name);
    // If the name isn't in the pool then there isn't a category with this name
    if (nameIndex < 0)
    {
        return -1;
    }

    NameKey key;
    key.kind = kind;
    key.nameIndex = nameIndex;

    if (auto ptr = m_nameMap.tryGetValue(key))
    {
        return *ptr;
    }

    return -1;
}

ConstArrayView<CommandOptions::Option> CommandOptions::getOptionsForCategory(Index categoryIndex) const
{
    const auto& cat = m_categories[categoryIndex];
    return makeConstArrayView(m_options.getBuffer() + cat.optionEndIndex, cat.optionEndIndex - cat.optionStartIndex);
}

void CommandOptions::getCategoryOptionNames(Index categoryIndex, List<UnownedStringSlice>& outNames) const
{
    outNames.clear();
    for (const auto& option : getOptionsForCategory(categoryIndex))
    {
        StringUtil::appendSplit(option.names, ',', outNames);
    }
}

void CommandOptions::findCategoryIndicesFromUsage(const UnownedStringSlice& slice, List<Index>& outCategories) const
{
    const auto* cur = slice.begin();
    const auto* end = slice.end();

    while (cur < end)
    {
        // Find < 
        while (cur < end && *cur != '<') cur++;

        // If we found it look for the end
        if (cur < end && *cur == '<')
        {
            ++cur;
            auto start = cur;
            while (cur < end && (CharUtil::isAlphaOrDigit(*cur) || *cur == '-' || *cur == '_') && *cur != '>')
            {
                cur++;
            }
            
            // If we hit closing > we want to lookup
            if (cur < end && *cur == '>')
            {
                const UnownedStringSlice categoryName(start, cur);

                Index categoryIndex = findCategoryByName(categoryName);
                if (categoryIndex >= 0 && outCategories.indexOf(categoryIndex) < 0)
                {
                    outCategories.add(categoryIndex);
                }
            }

            cur++;
        }
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! CommandOptionsWriter !!!!!!!!!!!!!!!!!!!!!!!!!!! */

void CommandOptionsWriter::appendDescription(const CommandOptions& options)
{
    // Go through categories in order

    const auto& categories = options.getCategories();

    for (Index categoryIndex = 0; categoryIndex < categories.getCount(); ++categoryIndex)
    {
        const auto& category = categories[categoryIndex];
        
        // Header
        {
            const auto count = m_builder.getLength();
            if (category.kind == CategoryKind::Value)
            {
                m_builder << "<" << category.name << ">";
            }
            else
            {
                m_builder << category.description;
            }
            const auto length = m_builder.getLength() - count;
            m_builder << "\n";

            m_builder.appendRepeatedChar('=', length);
       
            m_builder << "\n\n";
        }

        for (auto& option : options.getOptionsForCategory(categoryIndex))
        {
            m_builder << m_indentSlice;

            if (option.usage.getLength())
            {
                m_builder << option.usage;
            }
            else
            {
                List<UnownedStringSlice> names;
                StringUtil::split(option.names, ',', names);

                _appendWithWrap(1, names, toSlice(", "));
            }
           
            if (option.description.getLength() == 0)
            {
                m_builder << "\n";
                continue;
            }
            
            m_builder << ": ";

            List<UnownedStringSlice> lines;
            StringUtil::calcLines(option.description, lines);
                
            // Remove very last line if it's empty
            if (lines.getCount() > 1 && lines.getLast().trim().getLength() == 0)
            {
                lines.removeLast();
            }

            _appendWithWrap(2, lines);

            if (option.usage.getLength())
            {
                List<Index> usageCategoryIndices;
                options.findCategoryIndicesFromUsage(option.usage, usageCategoryIndices);

                for (auto usageCategoryIndex : usageCategoryIndices)
                {
                    auto& usageCat = categories[usageCategoryIndex];

                    m_builder << m_indentSlice << m_indentSlice;
                    m_builder << "<" << usageCat.name << "> can be: ";

                    List<UnownedStringSlice> optionNames;
                    options.getCategoryOptionNames(usageCategoryIndex, optionNames);

                    _appendWithWrap(2, optionNames, toSlice(", "));

                    m_builder << "\n";
                }
            }
        }

        m_builder << "\n";
    }
}

Count CommandOptionsWriter::_getCurrentLineLength()
{
    // Work out the current line length
    const char* start = m_builder.begin();
    const char* cur = m_builder.end();

    Count lineLength = 0;

    if (cur > start)
    {
        for (--cur; cur > start; --cur)
        {
            const auto c = *cur;
            if (c == '\n' || c == '\r')
            {
                ++cur;
                break;
            }
        }

        lineLength = Count(ptrdiff_t(m_builder.end() - cur));
    }

    return lineLength;
}

void CommandOptionsWriter::_requireIndent(Count indentCount)
{
    const auto length = m_builder.getLength();
    if (length)
    {
        const auto c = m_builder[length - 1];
        if (c == '\n' || c == '\r')
        {
            for (Index j = 0; j < indentCount; j++)
            {
                m_builder.append(m_indentSlice);
            }
        }
    }
}

void CommandOptionsWriter::_appendWithWrap(Count indentCount, List<UnownedStringSlice>& lines)
{
    List<UnownedStringSlice> words;

    for (auto line : lines)
    {
        if (line.trim().getLength() == 0 || line.startsWith(toSlice(" ")))
        {
            // Append the line as is after the indent
            _requireIndent(indentCount);
            m_builder << line << "\n";
        }
        else
        {
            words.clear();
            StringUtil::split(line, ' ', words);

            _requireIndent(indentCount);

            _appendWithWrap(indentCount, words, toSlice(" "));
            m_builder << "\n";
        }
    }
}

void CommandOptionsWriter::_appendWithWrap(Count indentCount, List<UnownedStringSlice>& slices, const UnownedStringSlice& delimit)
{
    Count lineLength = _getCurrentLineLength();

    const auto count = slices.getCount();

    for (Index i = 0; i < count; ++i)
    {
        auto slice = slices[i];

        auto sliceLength = slice.getLength();

        if (i < count - 1)
        {
            sliceLength += delimit.getLength();
        }

        // If out of space onto the next line
        if (lineLength + sliceLength > m_lineLength)
        {
            m_builder.append("\n");

            lineLength = indentCount * m_indentSlice.getLength();

            for (Index j = 0; j < indentCount; j++)
            {
                m_builder.append(m_indentSlice);
            }
        }

        m_builder.append(slice);
        if (i < count - 1)
        {
            m_builder.append(delimit);
        }

        lineLength += sliceLength;
    }
}

} // namespace Slang


