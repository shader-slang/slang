// slang-command-options-writer.cpp

#include "slang-command-options-writer.h"

#include "slang-string-util.h"
#include "slang-char-util.h"
#include "slang-byte-encode-util.h"

namespace Slang {
   
namespace { // anonymous
typedef CommandOptionsWriter::Style Style;
} // anonymous

static bool _isMarkdown(Style style) { return style == Style::Markdown || style == Style::NoLinkMarkdown; }
static bool _hasLinks(Style style) { return style == Style::Markdown; }

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MarkdownCommandOptionsWriter !!!!!!!!!!!!!!!!!!!!!!!!!!! */

class MarkdownCommandOptionsWriter : public CommandOptionsWriter
{
public:
    typedef CommandOptionsWriter Super;

    MarkdownCommandOptionsWriter(const Options& options):
        Super(options)
    {
    }

protected:
    // CommandOptionsWriter
    virtual void appendDescriptionForCategoryImpl(Index categoryIndex) SLANG_OVERRIDE;
    virtual void appendDescriptionImpl() SLANG_OVERRIDE;

    void _appendText(const UnownedStringSlice& text);
    void _appendDescriptionForCategory(Index categoryIndex);
    UnownedStringSlice _getLinkName(CommandOptions::LookupKind kind, Index index);

    bool m_hasLinks = false;
    Dictionary<NameKey, StringSlicePool::Handle> m_linkMap;
};

void MarkdownCommandOptionsWriter::appendDescriptionForCategoryImpl(Index categoryIndex)
{
    // No point doing links for a single category
    m_hasLinks = false;
    _appendDescriptionForCategory(categoryIndex);
}

void MarkdownCommandOptionsWriter::appendDescriptionImpl()
{
    m_hasLinks = _hasLinks(m_options.style);

    // Go through categories in order
    const auto& categories = m_commandOptions->getCategories();
    for (Index categoryIndex = 0; categoryIndex < categories.getCount(); ++categoryIndex)
    {
        _appendDescriptionForCategory(categoryIndex);
    }
}

static bool _needsMarkdownEscape(const UnownedStringSlice& text)
{
    for (auto c : text)
    {
        switch (c)
        {
            case '<':
            case '>':
            case '&':
            case '[':
            case ']':
            {
                return true;
            }
            default: break;
        }
    }

    return false;
}


void _appendEscapedMarkdown(const UnownedStringSlice& text, StringBuilder& ioBuf)
{
    if (_needsMarkdownEscape(text))
    {
        // Replace any < > &
        for (auto c : text)
        {
            switch (c)
            {
                case '<': ioBuf << "&lt;"; break;
                case '>': ioBuf << "&gt;"; break;
                case '&': ioBuf << "&amp;"; break;
                case '[': ioBuf << "\\["; break;
                case ']': ioBuf << "\\]"; break;
                default: ioBuf << c;
            }
        }
    }
    else
    {
        ioBuf << text;
    }
}


void MarkdownCommandOptionsWriter::_appendText(const UnownedStringSlice& text)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text, lines);
    for (auto line : lines)
    {
        if (line.startsWith(toSlice(" ")))
        {
            // If prefixed means we want to display as is
            m_builder << "> " << line << "\n";
        }
        else
        {
            _appendEscapedMarkdown(line, m_builder);
            m_builder << "\n\n";
        }
    }
}


void MarkdownCommandOptionsWriter::_appendDescriptionForCategory(Index categoryIndex)
{
    auto& options = *m_commandOptions;

    const auto& categories = options.getCategories();
    const auto& category = categories[categoryIndex];

    const bool isValue = (category.kind == CommandOptions::CategoryKind::Value);

    // Header
    {
        if (m_hasLinks)
        {
            // Output anchor
            m_builder << "<a id=\"" << _getLinkName(LookupKind::Category, categoryIndex) << "\"></a>\n";
        }

        m_builder << "# " << category.name << "\n\n";
      
        // If there is a description output, making \n split paragraphs
        if (category.description.getLength() > 0)
        {
            _appendText(category.description);
        } 
    }

    for (Index optionIndex = category.optionStartIndex; optionIndex < category.optionEndIndex; ++optionIndex)
    {
        const auto& option = options.getOptionAt(optionIndex);

        {
            List<UnownedStringSlice> names;
            StringUtil::split(option.names, ',', names);

            if (isValue)
            {
                m_builder << "* ";
                // Output all the names               
                m_builder << "`";
                StringUtil::join(names.getBuffer(), names.getCount(), toSlice("`, `"), m_builder);
                m_builder << "` ";
            }
            else
            {
                if (m_hasLinks)
                {
                    m_builder << "<a id=\"" << _getLinkName(LookupKind::Option, optionIndex) << "\"></a>\n";
                }

                m_builder << "## ";
                StringUtil::join(names.getBuffer(), names.getCount(), toSlice(", "), m_builder);
                m_builder << "\n";

                if (option.usage.getLength())
                {
                    m_builder << "\n**";

                    if (m_hasLinks)
                    {
                        List<UnownedStringSlice> usedCategories;
                        options.splitUsage(option.usage, usedCategories);

                        const char* cur = option.usage.begin();
                        for (auto usedCategory : usedCategories)
                        {
                            _appendEscapedMarkdown(UnownedStringSlice(cur, usedCategory.begin()), m_builder);

                            // Now do the link
                            const Index usedCategoryIndex = options.findCategoryByName(usedCategory);

                            m_builder << "[" << usedCategory << "](#" << _getLinkName(LookupKind::Category, usedCategoryIndex) << ")";

                            cur = usedCategory.end();
                        }

                        _appendEscapedMarkdown(UnownedStringSlice(cur, option.usage.end()), m_builder);
                    }
                    else
                    {
                        _appendEscapedMarkdown(option.usage, m_builder);
                    }

                    m_builder << "**\n\n";
                }
            }
        }

        if (option.description.getLength() > 0)
        {
            if (isValue)
            {
                m_builder << ": ";
                List<UnownedStringSlice> words;
                StringUtil::splitOnWhitespace(option.description, words);
                for (auto word : words)
                {
                    _appendEscapedMarkdown(word, m_builder);
                    m_builder << " ";
                }
            }
            else
            {
                _appendText(option.description);
            }
        }

        m_builder << "\n";
    }

    m_builder << "\n";
}

UnownedStringSlice MarkdownCommandOptionsWriter::_getLinkName(CommandOptions::LookupKind kind, Index index)
{
    NameKey key;
    key.kind = kind;
    key.nameIndex = index;

    if (auto ptr = m_linkMap.tryGetValue(key))
    {
        return m_pool.getSlice(*ptr);
    }

    auto& options = *m_commandOptions;

    UnownedStringSlice prefix;

    StringBuilder buf;
    if (kind == CommandOptions::LookupKind::Category)
    {
        prefix = options.getCategories()[index].name;
    }
    else if (kind == CommandOptions::LookupKind::Option)
    {
        prefix = StringUtil::getAtInSplit(UnownedStringSlice(options.getOptionAt(index).names), ',', 0);
    }

    if (prefix.startsWith(toSlice("-")))
    {
        buf << prefix.tail(1);
    }
    else
    {
        buf << prefix;
    }

    const auto bufLen = buf.getLength();

    for (Index i = 0; i < 1000; ++i)
    {
        buf.reduceLength(bufLen);

        if (i > 0)
        {
            buf << "-" << i;
        }

        if (!m_pool.has(buf.getUnownedSlice()))
        {
            break;
        }
    }

    const auto handle = m_pool.add(buf.getUnownedSlice());
    m_linkMap.add(key, handle);

    return m_pool.getSlice(handle);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! TextCommandOptionsWriter !!!!!!!!!!!!!!!!!!!!!!!!!!! */

class TextCommandOptionsWriter : public CommandOptionsWriter
{
public:
    typedef CommandOptionsWriter Super;
    
    TextCommandOptionsWriter(const Options& options) :
        Super(options)
    {
    }
protected:
    // CommandOptionsWriter
    virtual void appendDescriptionForCategoryImpl(Index categoryIndex) SLANG_OVERRIDE;
    virtual void appendDescriptionImpl() SLANG_OVERRIDE;

    void _appendText(Count indentCount, const UnownedStringSlice& text);
    void _appendDescriptionForCategory(Index categoryIndex);
};

void TextCommandOptionsWriter::appendDescriptionForCategoryImpl(Index categoryIndex)
{
    _appendDescriptionForCategory(categoryIndex);
}

void TextCommandOptionsWriter::appendDescriptionImpl()
{
    const auto& categories = m_commandOptions->getCategories();
    for (Index categoryIndex = 0; categoryIndex < categories.getCount(); ++categoryIndex)
    {
        _appendDescriptionForCategory(categoryIndex);
    }
}

void TextCommandOptionsWriter::_appendDescriptionForCategory(Index categoryIndex)
{
    auto& options = *m_commandOptions;

    const auto& categories = options.getCategories();
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
            m_builder << category.name;
        }

        const auto length = m_builder.getLength() - count;
        m_builder << "\n";

        m_builder.appendRepeatedChar('=', length);

        m_builder << "\n\n";

        // If there is a description output it
        if (category.description.getLength() > 0)
        {
            _appendText(0, category.description);
            m_builder << "\n";
        }
    }

    for (auto& option : options.getOptionsForCategory(categoryIndex))
    {
        m_builder << m_options.indent;

        if (option.usage.getLength())
        {
            m_builder << option.usage;
        }
        else
        {
            List<UnownedStringSlice> names;
            StringUtil::split(option.names, ',', names);

            _appendWrappedIndented(1, names, toSlice(", "));
        }

        if (option.description.getLength() == 0)
        {
            m_builder << "\n";
            continue;
        }

        m_builder << ": ";

        _appendText(2, option.description);

        if (option.usage.getLength())
        {
            List<Index> usageCategoryIndices;
            options.findCategoryIndicesFromUsage(option.usage, usageCategoryIndices);

            for (auto usageCategoryIndex : usageCategoryIndices)
            {
                auto& usageCat = categories[usageCategoryIndex];

                m_builder << m_options.indent << m_options.indent;

                m_builder << "<" << usageCat.name << "> can be: ";

                List<UnownedStringSlice> optionNames;
                options.getCategoryOptionNames(usageCategoryIndex, optionNames);

                _appendWrappedIndented(2, optionNames, toSlice(", "));

                m_builder << "\n";
            }
        }
    }

    m_builder << "\n";
}

void TextCommandOptionsWriter::_appendText(Count indentCount, const UnownedStringSlice& text)
{
    List<UnownedStringSlice> lines;
    StringUtil::calcLines(text, lines);

    // Remove very last line if it's empty
    if (lines.getCount() > 1 && lines.getLast().trim().getLength() == 0)
    {
        lines.removeLast();
    }

    List<UnownedStringSlice> words;

    for (auto line : lines)
    {
        if (line.startsWith(toSlice(" ")))
        {
            // Append the line as is after the indent
            _requireIndent(indentCount);
            m_builder << line;
        }
        else if (line.trim().getLength() == 0)
        {
        }
        else
        {
            words.clear();
            StringUtil::split(line, ' ', words);

            _requireIndent(indentCount);
            _appendWrappedIndented(indentCount, words, toSlice(" "));
        }

        m_builder << "\n";
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CommandOptionsWriter !!!!!!!!!!!!!!!!!!!!!!!!!!! */

typedef CommandOptionsWriter::Style Style;

static const NamesDescriptionValue s_styleInfos[] =
{
    { ValueInt(Style::Text), "text", "Text suitable for output to a terminal"  },
    { ValueInt(Style::Markdown), "markdown", "Markdown"  },
    { ValueInt(Style::NoLinkMarkdown), "no-link-markdown", "Markdown without links"  },
};

/* static */ConstArrayView<NamesDescriptionValue> CommandOptionsWriter::getStyleInfos()
{
    return makeConstArrayView(s_styleInfos);
}

CommandOptionsWriter::CommandOptionsWriter(const Options& options) :
    m_pool(StringSlicePool::Style::Default),
    m_options(options)
{
    m_options.indent = m_pool.addAndGetSlice(options.indent);
}

/* static */RefPtr<CommandOptionsWriter> CommandOptionsWriter::create(const Options& options)
{
    if (_isMarkdown(options.style))
    {
        return new MarkdownCommandOptionsWriter(options);
    }
    else
    {
        return new TextCommandOptionsWriter(options);
    }
}

void CommandOptionsWriter::appendDescriptionForCategory(CommandOptions* options, Index categoryIndex)
{
    m_commandOptions = options;
    appendDescriptionForCategoryImpl(categoryIndex);
    m_commandOptions = nullptr;
}

void CommandOptionsWriter::appendDescription(CommandOptions* options)
{
    m_commandOptions = options;
    appendDescriptionImpl();
    m_commandOptions = nullptr;
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
                m_builder.append(m_options.indent);
            }
        }
    }
}

void CommandOptionsWriter::_appendWrappedIndented(Count indentCount, List<UnownedStringSlice>& slices, const UnownedStringSlice& delimit)
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
        if (lineLength + sliceLength > m_options.lineLength)
        {
            m_builder.append("\n");

            lineLength = indentCount * m_options.indent.getLength();

            for (Index j = 0; j < indentCount; j++)
            {
                m_builder.append(m_options.indent);
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


