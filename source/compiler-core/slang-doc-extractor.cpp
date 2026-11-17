// slang-doc-extractor.cpp
#include "slang-doc-extractor.h"

#include "core/slang-string-util.h"

namespace Slang
{

// Documentation association searches backward or forward from a declaration through comments
// and punctuation. We keep those atomic lexemes in one ordered sequence so the search can cross
// token attachment boundaries without reconstructing a flattened view of tokens and their trivia.
// Whitespace and newlines are omitted; line relationships come from the source file's line map.

/* TODO(JS):

* If Decls hand SourceRange, then we could use the range to simplify getting the Post markup, as
will be trivial to get to the 'end'
* Need to handle preceeding * in some markup styles
* If we want to be able to disable markup we need a mechanism to do this. Probably define source
ranges.

* Need a way to take the extracted markup and produce suitable markdown
** This will need to display the decoration appropriately
*/

/* static */ UnownedStringSlice DocMarkupExtractor::removeStart(
    MarkupType type,
    const UnownedStringSlice& comment)
{
    switch (type)
    {
    case MarkupType::BlockBefore:
        {
            if (comment.startsWith(UnownedStringSlice::fromLiteral("/**")) ||
                comment.startsWith(UnownedStringSlice::fromLiteral("/*!")))
            {
                /// /**  */ or /*!  */.
                return comment.tail(3);
            }
            return comment;
        }
    case MarkupType::BlockAfter:
        {

            if (comment.startsWith(UnownedStringSlice::fromLiteral("/**<")) ||
                comment.startsWith(UnownedStringSlice::fromLiteral("/*!<")))
            {
                /// /*!< */ or /**< */
                return comment.tail(4);
            }
            return comment;
        }
    case MarkupType::OrdinaryBlockBefore:
        {
            if (comment.startsWith(UnownedStringSlice::fromLiteral("/*")))
            {
                /// ordinary /* */ block.
                return comment.tail(2);
            }
            return comment;
        }
    case MarkupType::LineBangBefore:
        {
            return comment.startsWith(UnownedStringSlice::fromLiteral("//!")) ? comment.tail(3)
                                                                              : comment;
        }
    case MarkupType::LineSlashBefore:
        {
            return comment.startsWith(UnownedStringSlice::fromLiteral("///")) ? comment.tail(3)
                                                                              : comment;
        }
    case MarkupType::OrdinaryLineBefore:
    case MarkupType::OrdinaryLineAfter:
        {
            return comment.startsWith(UnownedStringSlice::fromLiteral("//")) ? comment.tail(2)
                                                                             : comment;
        }
    case MarkupType::LineBangAfter:
        {
            /// //!< Can be multiple lines
            return comment.startsWith(UnownedStringSlice::fromLiteral("//!<")) ? comment.tail(4)
                                                                               : comment;
        }
    case MarkupType::LineSlashAfter:
        {
            return comment.startsWith(UnownedStringSlice::fromLiteral("///<")) ? comment.tail(4)
                                                                               : comment;
        }
    default:
        break;
    }
    return comment;
}

static LexemeList _readMarkupLexemes(Lexer& lexer)
{
    // We retain comments as searchable entries alongside raw tokens. We omit other trivia so
    // comments on consecutive lines can occupy adjacent entries in the search sequence.
    LexemeList lexemes(lexer.getTokenPool());
    for (;;)
    {
        auto lexeme = lexer.readLexeme();
        if (lexeme.is<RawToken>())
            lexemes.add(lexeme);
        else if (lexeme.is<CommentTrivia>())
            lexemes.add(lexeme);

        if (lexeme.is<RawEndOfFile>())
            return lexemes;
    }
}

static Index _findLexemeIndex(SourceLoc loc, const Lexeme* lexemes, Index lexemeCount)
{
    // The lexemes are ordered by location, so we search for an exact declaration location
    // without visiting every preceding comment and delimiter.
    Index lo = 0;
    Index hi = lexemeCount;

    while (lo + 1 < hi)
    {
        const Index mid = (hi + lo) >> 1;
        const Lexeme& midLexeme = lexemes[mid];

        if (midLexeme.getLoc() == loc)
        {
            return mid;
        }

        if (midLexeme.getLoc().getRaw() <= loc.getRaw())
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }

    return -1;
}

/* static */ DocMarkupExtractor::MarkupFlags DocMarkupExtractor::getFlags(MarkupType type)
{
    switch (type)
    {
    default:
    case MarkupType::None:
        return 0;
    case MarkupType::BlockBefore:
        return MarkupFlag::Before | MarkupFlag::IsBlock;
    case MarkupType::BlockAfter:
        return MarkupFlag::After | MarkupFlag::IsBlock;
    case MarkupType::OrdinaryBlockBefore:
        return MarkupFlag::Before | MarkupFlag::IsBlock;

    case MarkupType::LineBangBefore:
        return MarkupFlag::Before | MarkupFlag::AllowsMultipleComments;
    case MarkupType::LineSlashBefore:
        return MarkupFlag::Before | MarkupFlag::AllowsMultipleComments;
    case MarkupType::OrdinaryLineBefore:
        return MarkupFlag::Before | MarkupFlag::AllowsMultipleComments;

    case MarkupType::LineBangAfter:
        return MarkupFlag::After | MarkupFlag::AllowsMultipleComments;
    case MarkupType::LineSlashAfter:
        return MarkupFlag::After | MarkupFlag::AllowsMultipleComments;
    case MarkupType::OrdinaryLineAfter:
        return MarkupFlag::After | MarkupFlag::AllowsMultipleComments;
    }
}

/* static */ DocMarkupExtractor::MarkupType DocMarkupExtractor::findMarkupType(const Lexeme& lexeme)
{
    switch (lexeme.getTypeTag())
    {
    case LexemeTypeTag::BlockComment:
        {
            UnownedStringSlice slice = lexeme.getLogicalSpelling();
            if (slice.getLength() >= 3 && (slice[2] == '!' || slice[2] == '*'))
            {
                return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::BlockAfter
                                                                   : MarkupType::BlockBefore;
            }
            else
            {
                return MarkupType::OrdinaryBlockBefore;
            }
            break;
        }
    case LexemeTypeTag::LineComment:
        {
            UnownedStringSlice slice = lexeme.getLogicalSpelling();
            if (slice.getLength() >= 3)
            {
                if (slice[2] == '!')
                {
                    return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::LineBangAfter
                                                                       : MarkupType::LineBangBefore;
                }
                else if (slice[2] == '/')
                {
                    return (slice.getLength() >= 4 && slice[3] == '<')
                               ? MarkupType::LineSlashAfter
                               : MarkupType::LineSlashBefore;
                }
            }
            return (lexeme.getFlags() & TokenFlag::AtStartOfLine) != 0
                       ? MarkupType::OrdinaryLineBefore
                       : MarkupType::OrdinaryLineAfter;
            break;
        }
    default:
        break;
    }
    return MarkupType::None;
}

static Index _calcWhitespaceIndent(const UnownedStringSlice& line)
{
    // TODO(JS): For now we ignore tabs and just work out indentation based on spaces/assume ASCII
    Index indent = 0;
    const Index count = line.getLength();
    for (; indent < count && line[indent] == ' '; indent++)
        ;
    return indent;
}

static Index _calcIndent(const UnownedStringSlice& line)
{
    // TODO(JS): For now we just assume no tabs, and that every char is ASCII
    return line.getLength();
}

static void _appendUnindenttedLine(
    const UnownedStringSlice& line,
    Index maxIndent,
    StringBuilder& out)
{
    Index indent = _calcWhitespaceIndent(line);

    // We want to remove indenting remove no more than maxIndent
    if (maxIndent >= 0)
    {
        indent = (indent > maxIndent) ? maxIndent : indent;
    }

    // Remove the indenting, and append to out
    out.append(line.tail(indent));
}

SlangResult DocMarkupExtractor::_extractMarkup(
    const FindInfo& info,
    const FoundMarkup& foundMarkup,
    StringBuilder& out)
{
    SourceView* sourceView = info.sourceView;
    SourceFile* sourceFile = sourceView->getSourceFile();

    // We assemble documentation text from the selected comment lexemes.
    // We remove the comment delimiters and preserve indentation within the comment body.

    switch (foundMarkup.type)
    {
    case MarkupType::BlockBefore:
    case MarkupType::BlockAfter:
    case MarkupType::OrdinaryBlockBefore:
        {
            // We should only have a single line
            SLANG_ASSERT(foundMarkup.range.getCount() == 1);

            const auto& lexeme = info.lexemeList->getLexemes()[foundMarkup.range.start];
            uint32_t offset = sourceView->getRange().getOffset(lexeme.getLoc());

            const UnownedStringSlice startLine = sourceFile->getLineContainingOffset(offset);

            UnownedStringSlice content = lexeme.getLogicalSpelling();

            // Split into lines
            List<UnownedStringSlice> lines;

            StringUtil::calcLines(content, lines);

            Index maxIndent = -1;

            StringBuilder unindentedLine;

            const Index linesCount = lines.getCount();
            for (Index i = 0; i < linesCount; ++i)
            {
                UnownedStringSlice line = lines[i];
                unindentedLine.clear();

                if (i == 0)
                {
                    // The pool owns the logical spelling separately from the file bytes.
                    // We measure indentation at the comment's physical position and remove
                    // the opener from the logical text used for extraction.
                    const char* commentBegin = sourceFile->getContent().begin() + offset;
                    SLANG_RELEASE_ASSERT(startLine.isMemoryContained(commentBegin));
                    maxIndent = _calcIndent(UnownedStringSlice(startLine.begin(), commentBegin));
                    line = removeStart(foundMarkup.type, line);
                }

                if (i == linesCount - 1)
                {
                    SLANG_ASSERT(
                        line.tail(line.getLength() - 2) == UnownedStringSlice::fromLiteral("*/"));
                    // Remove the */ at the end of the line
                    line = line.head(line.getLength() - 2);
                }

                if (i > 0)
                {
                    _appendUnindenttedLine(line, maxIndent, unindentedLine);
                }
                else
                {
                    unindentedLine.append(line);
                }

                // If the first or last line are all white space, just ignore them
                if ((i == linesCount - 1 || i == 0) &&
                    unindentedLine.getUnownedSlice().trim().getLength() == 0)
                {
                    continue;
                }

                out.append(unindentedLine);
                out.appendChar('\n');
            }

            break;
        }
    case MarkupType::OrdinaryLineBefore:
    case MarkupType::OrdinaryLineAfter:
    case MarkupType::LineBangBefore:
    case MarkupType::LineSlashBefore:
    case MarkupType::LineBangAfter:
    case MarkupType::LineSlashAfter:
        {
            // Holds the lines extracted, they may have some white space indenting (like the space
            // at the start of //)
            List<UnownedStringSlice> lines;

            const auto& range = foundMarkup.range;
            for (Index i = range.start; i < range.end; ++i)
            {
                const auto& lexeme = info.lexemeList->getLexemes()[i];
                UnownedStringSlice line = lexeme.getLogicalSpelling();
                line = removeStart(foundMarkup.type, line);

                // If the first or last line are all white space, just ignore them
                if ((i == range.start || i == range.end - 1) && line.trim().getLength() == 0)
                {
                    continue;
                }
                lines.add(line);
            }

            if (lines.getCount() == 0)
            {
                // If there are no lines, theres no content
                return SLANG_OK;
            }

            Index minIndent = 0x7fffffff;
            for (const auto& line : lines)
            {
                const Index indent = _calcWhitespaceIndent(line);
                minIndent = (indent < minIndent) ? indent : minIndent;
            }

            for (const auto& line : lines)
            {
                _appendUnindenttedLine(line, minIndent, out);
                out.appendChar('\n');
            }

            break;
        }
    default:
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

Index DocMarkupExtractor::_findStartIndex(const FindInfo& info, Location location)
{
    // We search in the direction selected by `location`, crossing balanced delimiters to
    // avoid associating a declaration with comments inside a nested parameter or expression.
    // At the declaration's delimiter depth, punctuation determines where a comment may begin.
    Index openCount = 0;

    const LexemeList& lexemes = *info.lexemeList;
    const Index lexemeIndex = info.lexemeIndex;

    Index direction = isBefore(location) ? -1 : 1;

    const Index count = lexemes.getLexemes().getCount();
    for (Index i = lexemeIndex; i >= 0 && i < count; i += direction)
    {
        const Lexeme& lexeme = lexemes.getLexemes()[i];

        switch (lexeme.getTypeTag())
        {
        case LexemeTypeTag::LBrace:
        case LexemeTypeTag::LBracket:
        case LexemeTypeTag::LParent:
        case LexemeTypeTag::OpLess:
            {
                openCount += direction;
                if (openCount < 0)
                    return -1;
                break;
            }
        case LexemeTypeTag::RBracket:
            {
                openCount -= direction;
                if (openCount < 0)
                    return -1;
                break;
            }
        case LexemeTypeTag::OpGreater:
            {
                if (location == Location::AfterGenericParam && openCount == 0)
                {
                    return i + 1;
                }

                openCount -= direction;
                if (openCount < 0)
                    return -1;

                break;
            }
        case LexemeTypeTag::RParent:
            {
                if (openCount == 0 && location == Location::AfterParam)
                {
                    return i + 1;
                }

                openCount -= direction;
                if (openCount < 0)
                    return -1;
                break;
            }
        case LexemeTypeTag::RBrace:
            {
                // If we haven't hit a candidate yet before hitting } it's not going to work
                if (location == Location::Before || location == Location::AfterEnumCase)
                {
                    return -1;
                }
                break;
            }
        case LexemeTypeTag::BlockComment:
        case LexemeTypeTag::LineComment:
            {
                if (openCount == 0)
                {
                    // Determine the markup type
                    const MarkupType markupType = findMarkupType(lexeme);
                    if (!m_searchInOrindaryComments &&
                        (markupType == MarkupType::OrdinaryBlockBefore ||
                         markupType == MarkupType::OrdinaryLineBefore))
                        break;
                    // If the location wanted is before and the markup is, we'll assume this is it
                    if (isBefore(location) && isBefore(markupType))
                    {
                        return i;
                    }
                    // If we are looking for enum cases, and the markup is after, we'll assume this
                    // is it
                    if (isAfter(location) && isAfter(markupType))
                    {
                        return i;
                    }
                }
                break;
            }
        case LexemeTypeTag::Comma:
            {
                if (openCount == 0)
                {
                    if (location == Location::AfterParam || location == Location::AfterEnumCase ||
                        location == Location::AfterGenericParam)
                    {
                        return i + 1;
                    }

                    if (location == Location::Before)
                    {
                        return -1;
                    }
                }

                break;
            }
        case LexemeTypeTag::Semicolon:
            {
                // If we haven't hit a candidate yet it's not going to work
                if (location == Location::Before)
                {
                    return -1;
                }
                if (openCount == 0 && location == Location::AfterSemicolon)
                {
                    return i + 1;
                }
                break;
            }
        default:
            break;
        }
    }

    return -1;
}

/* static */ bool DocMarkupExtractor::_isLexemeOnLineIndex(
    SourceView* sourceView,
    MarkupType type,
    const Lexeme& lexeme,
    Index lineIndex)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    const int offset = sourceView->getRange().getOffset(lexeme.getLoc());

    auto const flags = getFlags(type);

    if (flags & MarkupFlag::IsBlock)
    {
        // We accept a block comment when either end touches the requested line. The end
        // location uses the physical spelling length: escaped newlines are absent from the
        // logical spelling, but their bytes still separate the start and end in the input file.
        const auto physicalLength = lexeme.getPhysicalSpellingLength();
        SLANG_RELEASE_ASSERT(physicalLength <= UINT32_MAX - uint32_t(offset));
        const auto endOffset = uint32_t(offset) + uint32_t(physicalLength);
        return sourceFile->isOffsetOnLine(offset, lineIndex) ||
               sourceFile->isOffsetOnLine(endOffset, lineIndex);
    }
    else
    {
        // Has to be exactly on the specified line
        return sourceFile->isOffsetOnLine(offset, lineIndex);
    }
}

SlangResult DocMarkupExtractor::_findMarkup(
    const FindInfo& info,
    Location location,
    FoundMarkup& out)
{
    out.reset();

    const auto& lexemes = info.lexemeList->getLexemes();

    // We first locate a candidate comment by searching past balanced delimiters.
    Index startIndex = _findStartIndex(info, location);
    if (startIndex <= 0)
    {
        return SLANG_E_NOT_FOUND;
    }

    SourceView* sourceView = info.sourceView;
    SourceFile* sourceFile = sourceView->getSourceFile();

    // Let's lookup the line index where this occurred
    const int startOffset = sourceView->getRange().getOffset(lexemes[startIndex].getLoc());

    // The line index that the markoff starts from
    Index lineIndex = sourceFile->calcLineIndexFromOffset(startOffset);
    if (lineIndex < 0)
    {
        return SLANG_E_NOT_FOUND;
    }

    const Index searchDirection = isBefore(location) ? -1 : 1;

    // Get the type and flags
    const MarkupType type = findMarkupType(lexemes[startIndex]);
    const MarkupFlags flags = getFlags(type);

    const MarkupFlag::Enum requiredFlag =
        isBefore(location) ? MarkupFlag::Before : MarkupFlag::After;
    if ((flags & requiredFlag) == 0)
    {
        return SLANG_E_NOT_FOUND;
    }

#if 0
    // The lexeme still isn't accepted, unless it's on the expected line.
    if (_isLexemeOnLineIndex(info.sourceView, type, lexemes[startIndex], expectedLineIndex))
    {
        return SLANG_E_NOT_FOUND;
    }
#endif

    Index endIndex = startIndex;

    // If it's multiline, so look for the end index
    if (flags & MarkupFlag::AllowsMultipleComments)
    {
        Index expectedLineIndex = lineIndex;

        // TODO(JS):
        // We should probably do the work here to confirm  indentation - but that
        // requires knowing something about tabs, so for now we leave.

        while (true)
        {
            endIndex += searchDirection;
            expectedLineIndex += searchDirection;
            if (expectedLineIndex < 0)
                break;
            if (endIndex < 0 || endIndex >= lexemes.getCount())
            {
                break;
            }

            // Consecutive documentation lines must use the same comment style.
            if (findMarkupType(lexemes[endIndex]) != type)
            {
                break;
            }

            // Is it on the right line?
            if (!_isLexemeOnLineIndex(info.sourceView, type, lexemes[endIndex], expectedLineIndex))
            {
                break;
            }
        }

        // Fix the end index (it's the last one that worked)
        endIndex -= searchDirection;
    }

    // Put start < end order
    if (endIndex < startIndex)
    {
        Swap(endIndex, startIndex);
    }
    // The range excludes end so increase
    endIndex++;

    // Okay we've found the markup
    out.type = type;
    out.location = location;
    out.range = IndexRange{startIndex, endIndex};

    SLANG_ASSERT(out.range.getCount() > 0);

    return SLANG_OK;
}

SlangResult DocMarkupExtractor::_findFirstMarkup(
    const FindInfo& info,
    const Location* locs,
    Index locCount,
    FoundMarkup& out,
    Index& outIndex)
{
    Index i = 0;
    for (; i < locCount; ++i)
    {
        SlangResult res = _findMarkup(info, locs[i], out);
        if (SLANG_SUCCEEDED(res) || (SLANG_FAILED(res) && res != SLANG_E_NOT_FOUND))
        {
            outIndex = i;
            return res;
        }
    }
    return SLANG_E_NOT_FOUND;
}

SlangResult DocMarkupExtractor::_findMarkup(
    const FindInfo& info,
    const Location* locs,
    Index locCount,
    FoundMarkup& out)
{
    Index foundIndex;
    SLANG_RETURN_ON_FAIL(_findFirstMarkup(info, locs, locCount, out, foundIndex));

    // Lets see if the remaining ones match
    {
        FoundMarkup otherMarkup;
        for (Index i = foundIndex + 1; i < locCount; ++i)
        {
            SlangResult res = _findMarkup(info, locs[i], otherMarkup);
            if (SLANG_SUCCEEDED(res))
            {
                // TODO(JS): Warning found markup in another location
            }
        }
    }

    return SLANG_OK;
}


SlangResult DocMarkupExtractor::_findMarkup(
    const FindInfo& info,
    SearchStyle searchStyle,
    FoundMarkup& out)
{
    switch (searchStyle)
    {
    default:
    case SearchStyle::None:
        {
            return SLANG_E_NOT_FOUND;
        }
    case SearchStyle::EnumCase:
        {
            Location locs[] = {Location::Before, Location::AfterEnumCase};
            return _findMarkup(info, locs, SLANG_COUNT_OF(locs), out);
        }
    case SearchStyle::Param:
        {
            Location locs[] = {Location::Before, Location::AfterParam};
            return _findMarkup(info, locs, SLANG_COUNT_OF(locs), out);
        }
    case SearchStyle::Before:
        {
            return _findMarkup(info, Location::Before, out);
        }
    case SearchStyle::Function:
        {
            return _findMarkup(info, Location::Before, out);
        }
    case SearchStyle::Attribute:
        {
            FindInfo newInfo = info;
            newInfo.lexemeIndex -= 2;
            return _findMarkup(newInfo, Location::Before, out);
        }
    case SearchStyle::Variable:
        {
            Location locs[] = {Location::Before, Location::AfterSemicolon};
            return _findMarkup(info, locs, SLANG_COUNT_OF(locs), out);
        }
    case SearchStyle::GenericParam:
        {
            Location locs[] = {Location::Before, Location::AfterGenericParam};
            return _findMarkup(info, locs, SLANG_COUNT_OF(locs), out);
        }
    }
}

static void _calcLineVisibility(
    SourceView* sourceView,
    const LexemeList& lexemes,
    List<MarkupVisibility>& outLineVisibility)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    const auto& lineOffsets = sourceFile->getLineBreakOffsets();

    outLineVisibility.setCount(lineOffsets.getCount() + 1);

    MarkupVisibility lastVisibility = MarkupVisibility::Public;
    Index lastLine = 0;

    for (const auto& lexeme : lexemes)
    {
        if (lexeme.getTypeTag() == LexemeTypeTag::LineComment)
        {
            UnownedStringSlice contents = lexeme.getLogicalSpelling();

            MarkupVisibility newVisibility = lastVisibility;

            // Distinct from other markup
            if (contents.startsWith(toSlice("//@")))
            {
                UnownedStringSlice access = contents.tail(3).trim();
                if (access == "hidden:" || access == "private:")
                {
                    newVisibility = MarkupVisibility::Hidden;
                }
                else if (access == "internal:")
                {
                    newVisibility = MarkupVisibility::Internal;
                }
                else if (access == "public:")
                {
                    newVisibility = MarkupVisibility::Public;
                }
            }

            if (newVisibility != lastVisibility)
            {
                // Work up the line it's on
                const int offset = sourceView->getRange().getOffset(lexeme.getLoc());
                Index line = sourceFile->calcLineIndexFromOffset(offset);

                // Fill in the span
                for (Index i = lastLine; i < line; ++i)
                {
                    outLineVisibility[i] = lastVisibility;
                }

                // Record the new access and where we are up to
                lastLine = line;
                lastVisibility = newVisibility;
            }
        }
    }

    // Fill in the remaining
    for (Index i = lastLine; i < outLineVisibility.getCount(); ++i)
    {
        outLineVisibility[i] = lastVisibility;
    }
}

SlangResult DocMarkupExtractor::extract(
    const SearchItemInput* inputs,
    Index inputCount,
    SourceManager* sourceManager,
    DiagnosticSink* sink,
    List<SourceView*>& outViews,
    List<SearchItemOutput>& out)
{
    struct Entry
    {
        Index viewIndex;                 ///< The view/file index this loc is found in
        SourceLoc::RawValue locOrOffset; ///< Can be a loc or an offset into the file

        SearchStyle searchStyle; ///< The search style when looking for an item
        Index inputIndex;        ///< The index to this item in the input
    };

    List<Entry> entries;

    {
        entries.setCount(inputCount);
        for (Index i = 0; i < inputCount; ++i)
        {
            const auto& input = inputs[i];
            Entry& entry = entries[i];
            entry.inputIndex = i;
            entry.viewIndex = -1; //< We don't know what file/view it's in
            entry.locOrOffset = input.sourceLoc.getRaw();
            entry.searchStyle = input.searchStyle;
        }
    }

    // Sort them into loc order
    entries.sort(
        [](const Entry& a, const Entry& b) -> bool { return a.locOrOffset < b.locOrOffset; });

    {
        SourceView* sourceView = nullptr;
        Index viewIndex = -1;

        for (auto& entry : entries)
        {
            if (entry.searchStyle == SearchStyle::None)
            {
                continue;
            }

            const SourceLoc loc = SourceLoc::fromRaw(entry.locOrOffset);

            if (sourceView == nullptr || !sourceView->getRange().contains(loc))
            {
                // Find the new view
                sourceView = sourceManager->findSourceView(loc);
                if (!sourceView)
                {
                    entry.searchStyle = SearchStyle::None;
                    continue;
                }

                // We want only one view per SourceFile
                SourceFile* sourceFile = sourceView->getSourceFile();

                // NOTE! The view found might be different than sourceView.
                viewIndex = outViews.findFirstIndex(
                    [&](SourceView* currentView) -> bool
                    { return currentView->getSourceFile() == sourceFile; });

                if (viewIndex < 0)
                {
                    viewIndex = outViews.getCount();
                    outViews.add(sourceView);
                }
            }

            SLANG_ASSERT(viewIndex >= 0);
            SLANG_ASSERT(sourceView && sourceView->getRange().contains(loc));

            // Set the file index
            entry.viewIndex = viewIndex;
            // Set as the offset within the file
            entry.locOrOffset = sourceView->getRange().getOffset(loc);
        }

        // Sort into view/file and then offset order
        entries.sort(
            [](const Entry& a, const Entry& b) -> bool
            {
                return (a.viewIndex < b.viewIndex) ||
                       ((a.viewIndex == b.viewIndex) && a.locOrOffset < b.locOrOffset);
            });
    }

    {
        LexemeList lexemes;
        List<MarkupVisibility> lineVisibility;

        NamePool namePool;

        Index viewIndex = -1;
        SourceView* sourceView = nullptr;

        const Int entryCount = entries.getCount();

        out.setCount(entryCount);

        for (Index i = 0; i < entryCount; ++i)
        {
            const auto& entry = entries[i];
            auto& dst = out[i];

            dst.viewIndex = -1;
            dst.inputIndex = entry.inputIndex;
            dst.visibilty = MarkupVisibility::Public;

            // If there isn't a mechanism to search with, just move on
            if (entry.searchStyle == SearchStyle::None)
            {
                continue;
            }

            if (viewIndex != entry.viewIndex)
            {
                viewIndex = entry.viewIndex;
                sourceView = outViews[viewIndex];

                // Run the lexer
                Lexer lexer;
                RefPtr<TokenPool> tokenPool = new TokenPool();
                lexer.initialize(sourceView, sink, &namePool, tokenPool);

                lexemes = _readMarkupLexemes(lexer);

                _calcLineVisibility(sourceView, lexemes, lineVisibility);
            }

            dst.viewIndex = viewIndex;

            // Get the offset within the source file
            const uint32_t offset = entry.locOrOffset;

            // We convert the file offset to a location in `sourceView`, matching the locations
            // recorded by the lexer for the comment search.
            const SourceLoc loc = sourceView->getRange().getSourceLocFromOffset(offset);

            // Work out the line number
            SourceFile* sourceFile = sourceView->getSourceFile();
            const Index lineIndex = sourceFile->calcLineIndexFromOffset(int(offset));

            dst.visibilty = lineVisibility[lineIndex];

            // We start the comment search at the declaration's exact lexeme location.
            Index lexemeIndex = _findLexemeIndex(
                loc,
                lexemes.getLexemes().getBuffer(),
                lexemes.getLexemes().getCount());
            if (lexemeIndex >= 0 && lineIndex >= 0)
            {
                FindInfo findInfo;
                findInfo.lexemeIndex = lexemeIndex;
                findInfo.lineIndex = lineIndex;
                findInfo.lexemeList = &lexemes;
                findInfo.sourceView = sourceView;

                // Okay let's see if we extract some documentation then for this.
                FoundMarkup foundMarkup;
                SlangResult res = _findMarkup(findInfo, entry.searchStyle, foundMarkup);

                if (SLANG_SUCCEEDED(res))
                {
                    // We need to extract
                    StringBuilder buf;
                    SLANG_RETURN_ON_FAIL(_extractMarkup(findInfo, foundMarkup, buf));

                    // Save the extracted text in the output
                    dst.text = buf;
                }
                else if (res != SLANG_E_NOT_FOUND)
                {
                    return res;
                }
            }
        }
    }

    return SLANG_OK;
}

} // namespace Slang
