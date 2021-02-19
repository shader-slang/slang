// slang-doc.cpp
#include "slang-doc.h"

namespace Slang {

/* TODO(JS):

* If Decls hand SourceRange, then we could use the range to simplify getting the Post markup, as will be trivial to get to the 'end'
* Need a mechanism to extract the markup
** Need to handle preceeding * in some markup styles
* Still need a way to check that markup line numbers, such that block markup is on preceeding lines
** May want to handle the case that indentation is the same - we have to probably ignore if we have tabs tho
* If we want to be able to disable markup we need a mechanism to do this. Probably define source ranges.

* Need a way to take the extracted markup and produce suitable markdown
** This will need to display the decoration appropriately

* We will probably want to output the declarations in *source* order

*/

class DocumentationContext 
{
public:

    typedef uint32_t MarkupFlags;
    struct MarkupFlag
    {
        enum Enum : MarkupFlags
        {
            Before          = 0x1,
            After           = 0x2,
            IsMultiToken    = 0x4,          ///< Can use more than one token
            IsBlock         = 0x8,          ///< 
        };
    };

    enum class MarkupType
    {
        None,
        BlockBefore,        /// /**  */ or /*!  */.
        BlockAfter,         /// /*!< */ or /**< */

        LineBangBefore,      /// //! Can be multiple lines
        LineSlashBefore,     /// /// Can be multiple lines

        LineBangAfter,       /// //!< Can be multiple lines
        LineSlashAfter,      /// ///< Can be multiple lines
    };

    struct IndexRange
    {
        SLANG_FORCE_INLINE Index getCount() const { return end - start; }

        Index start;
        Index end;
    };

    enum class Location
    {
        None,                           ///< No defined location
        Before,
        AfterParam,                     ///< Can have trailing , or )
        AfterSemicolon,                 ///< Can have a trailing ;
    };

    struct FoundMarkup
    {
        void reset()
        {
            location = Location::None;
            type = MarkupType::None;
            range = IndexRange { 0, 0 };
        }

        Location location = Location::None;
        MarkupType type = MarkupType::None;
        IndexRange range;
    };

    struct FindInfo
    {

        SourceView* sourceView;         ///< The source view the tokens were generated from
        TokenList* tokenList;           ///< The token list
        Index declTokenIndex;           ///< The token index location (where searches start from)
        Index declLineIndex;            ///< The line number for the decl
    };

    SlangResult extract(Documentation* doc, ModuleDecl* moduleDecl, SourceManager* sourceManager, DiagnosticSink* sink);

    static MarkupFlags getFlags(MarkupType type);
    static MarkupType findMarkupType(const Token& tok);

protected:
        /// returns SLANG_E_NOT_FOUND if not found, SLANG_OK on success else an error
    SlangResult _findMarkup(const FindInfo& info, Location location, FoundMarkup& out);

        /// True if the line ordering is appropriate (no gaps, starts appropriately)
    bool _isLineOrderingOk(const FindInfo& info, const FoundMarkup& found, Index lineIndex);

        /// Locations are processed in order, and the first successful used. If found in another location will issue a warning.
        /// returns SLANG_E_NOT_FOUND if not found, SLANG_OK on success else an error
    SlangResult _findFirstMarkup(const FindInfo& info, const Location* locs, Index locCount, FoundMarkup& out, Index& outIndex);

    SlangResult _findMarkup(const FindInfo& info, const Location* locs, Index locCount, FoundMarkup& out);

        /// Given the decl, the token stream, and the decls tokenIndex, try to find some associated markup
    SlangResult _findMarkup(const FindInfo& info, Decl* decl, FoundMarkup& out);

        /// Given a found markup location extracts the contents of the tokens into out
    SlangResult _extractMarkup(const FindInfo& info, const FoundMarkup& foundMarkup, StringBuilder& out);

        /// Given a location, try to find the first token index that could potentially be markup
        /// Only works on 'after' locations, and will return -1 if not found
    Index _findAfterIndex(const FindInfo& info, Location location);

    static bool _isLineIndexOk(SourceView* sourceView, MarkupType type, Location location, const Token& tok, Index lineIndex);

    void _addDecl(Decl* decl);
    void _addDeclRec(Decl* decl);
    void _findDecls(ModuleDecl* moduleDecl);
    
    List<Decl*> m_decls;

    Documentation* m_doc;
    ModuleDecl* m_moduleDecl;
    SourceManager* m_sourceManager;
    DiagnosticSink* m_sink;
};


/*
EnumDecl
    EnumCaseDecl

StructDecl
ClassDecl

The decls in general in slang-ast-decl.h

We mainly care about decls that are in 'global' scope.
We probably don't care for example about variable/type declarations in a function.

*/

void DocumentationContext::_addDecl(Decl* decl)
{
    if (!decl->loc.isValid())
    {
        return;
    }
    m_decls.add(decl);
}

void DocumentationContext::_addDeclRec(Decl* decl)
{
#if 0
    if (CallableDecl* callableDecl = as<CallableDecl>(decl))
    {
        // For callables (like functions),

        m_decls.add(callableDecl);
    }
    else
#endif
    if (ContainerDecl* containerDecl = as<ContainerDecl>(decl))
    {
        // Add the container - which could be a class, struct, enum, namespace, extension, generic etc.
        _addDecl(decl);

        // Now add what the container contains
        for (Decl* childDecl : containerDecl->members)
        {
            _addDeclRec(childDecl);
        }
    }
    else 
    {
        // Just add.
        // There may be things we don't want to add, but just add them all of now
        _addDecl(decl);
    }
}

void DocumentationContext::_findDecls(ModuleDecl* moduleDecl)
{
    for (Decl* decl : moduleDecl->members)
    {
        _addDecl(decl);
    }
}

static Index _findTokenIndex(SourceLoc loc, const Token* toks, Index numToks)
{
    // Use a binary search to find the token
    Index lo = 0;
    Index hi = numToks;

    while (lo + 1 < hi)
    {
        const Index mid = (hi + lo) >> 1;
        const Token& midToken = toks[mid];

        if (midToken.loc == loc)
        {
            return mid;
        }

        if (midToken.loc.getRaw() <= loc.getRaw())
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }

    // Not found
    return -1;
}

/* static */DocumentationContext::MarkupFlags DocumentationContext::getFlags(MarkupType type)
{
    switch (type)
    {
        default:
        case MarkupType::None:              return 0;
        case MarkupType::BlockBefore:      return MarkupFlag::Before | MarkupFlag::IsBlock; 
        case MarkupType::BlockAfter:       return MarkupFlag::After  | MarkupFlag::IsBlock;

        case MarkupType::LineBangBefore:     return MarkupFlag::Before | MarkupFlag::IsMultiToken; 
        case MarkupType::LineSlashBefore:    return MarkupFlag::Before | MarkupFlag::IsMultiToken; 

        case MarkupType::LineBangAfter:      return MarkupFlag::After | MarkupFlag::IsMultiToken; 
        case MarkupType::LineSlashAfter:     return MarkupFlag::After | MarkupFlag::IsMultiToken;
    }
}

/* static */DocumentationContext::MarkupType DocumentationContext::findMarkupType(const Token& tok)
{
    switch (tok.type)
    {
        case TokenType::BlockComment:
        {
            UnownedStringSlice slice = tok.getContent();
            if (slice.getLength() >= 3 && (slice[2] == '!' || slice[2] == '*'))
            {
                return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::BlockAfter : MarkupType::BlockBefore;
            }
            break;
        }
        case TokenType::LineComment:
        {
            UnownedStringSlice slice = tok.getContent();
            if (slice.getLength() >= 3)
            {
                if (slice[2] == '!')
                {
                    return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::LineBangAfter : MarkupType::LineBangBefore;
                }
                else if (slice[2] == '/')
                {
                    return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::LineSlashAfter : MarkupType::LineSlashBefore;
                }
            }
            break;
        }
        default: break;
    }
    return MarkupType::None;
}

SlangResult DocumentationContext::_extractMarkup(const FindInfo& info, const FoundMarkup& foundMarkup, StringBuilder& out)
{
    SLANG_UNUSED(info);
    SLANG_UNUSED(foundMarkup);

    SLANG_UNUSED(out);
    return SLANG_FAIL;
}

Index DocumentationContext::_findAfterIndex(const FindInfo& info, Location location)
{
    Index openParensCount = 0;

    const TokenList& toks = *info.tokenList;
    const Index tokIndex = info.declTokenIndex;

    const Index count = toks.m_tokens.getCount();
    for (Index i = tokIndex; i < count; ++i)
    {
        const Token& tok = toks.m_tokens[i];

        switch (tok.type)
        {
            case TokenType::LParent:
            {
                ++openParensCount;
                break;
            }
            case TokenType::RParent:
            {
                if (openParensCount == 0 &&
                    location == Location::AfterParam)
                {
                    return i + 1;
                }

                --openParensCount;
                if (openParensCount < 0)
                {
                    // Not found - or weird parens at least 
                    return -1;
                }
                break;
            }
            case TokenType::Comma:
            {
                if (location == Location::AfterParam)
                {
                    return i + 1;
                }
                break;
            }
            case TokenType::Semicolon:
            {
                if (openParensCount == 0 && location == Location::AfterSemicolon)
                {
                    return i + 1;
                }
                break;
            }
            default: break;
        }
    }

    return -1;
}

static Index _findLineIndex(SourceFile* sourceFile, Index lineIndex, uint32_t offset)
{
    const List<uint32_t>& offsets = sourceFile->getLineBreakOffsets();
    const Index searchSize = 10;
    const uint32_t lineOffset = offsets[lineIndex];

    if (offset > lineOffset)
    {
        Index lastLine = lineIndex - searchSize;
        lastLine = (lastLine < 0) ? 0 : lastLine;
        // Search for first line where the offset is less
        for (Index i = lineIndex - 1; i >= lastLine; --i)
        {
            if (offsets[i] < offset)
            {
                return i + 1;
            }
        }
    }
    else
    {
        Index lastLine = lineIndex + searchSize;
        lastLine = (lastLine > offsets.getCount()) ? offsets.getCount() : lastLine;

        // Search for first line where it is greater
        for (Index i = lineIndex + 1; i < lastLine; ++i)
        {
            if (offsets[i] > offset)
            {
                return i - 1;
            }
        }
    }
    // Do it the slower way
    return sourceFile->calcLineIndexFromOffset(offset);
}

bool DocumentationContext::_isLineOrderingOk(const FindInfo& info, const FoundMarkup& found, Index lineIndex)
{
    const Index toksCount = found.range.getCount();
    if (toksCount <= 0)
    {
        return false;
    }

    const MarkupFlags flags = getFlags(found.type);  
    if ((flags & MarkupFlag::IsMultiToken) && toksCount > 1)
    {
        return false;
    }

    SourceView* sourceView = info.sourceView;
    SourceFile* sourceFile = sourceView->getSourceFile();
    const auto& locRange = sourceView->getRange();

    if (flags & MarkupFlag::IsBlock)
    {
        Token& tok = info.tokenList->m_tokens[found.range.start];
        int offset = locRange.getOffset(tok.loc);

        if (found.location == Location::Before)
        {
            // The end of the token should be on the line before
            return sourceFile->isOffsetOnLine(offset + tok.charsCount, lineIndex - 1);
        }
        else
        {
            // Has to start on the same line
            return sourceFile->isOffsetOnLine(offset, lineIndex);
        }
    }
    else
    {
        for (Index i = 0; i < toksCount; ++i)
        {
            const Token& tok = info.tokenList->m_tokens[found.range.start + i];
            int offset = locRange.getOffset(tok.loc);

            // If before, all lines have to be one line before
            // If after all lines start on lineIndex
            const Index expectedLineIndex = (found.location == Location::Before) ? ((lineIndex - toksCount) + i) : (lineIndex + i);

            if (!sourceFile->isOffsetOnLine(offset, lineIndex + i))
            {
                return false;
            }
        }
    }

    return true;
}

/* static */bool DocumentationContext::_isLineIndexOk(SourceView* sourceView, MarkupType type, Location location, const Token& tok, Index lineIndex)
{
    SourceFile* sourceFile = sourceView->getSourceFile();
    const auto& locRange = sourceView->getRange();

    int offset = locRange.getOffset(tok.loc);

    auto const flags = getFlags(type);

    if (flags & MarkupFlag::IsBlock)
    {
        // Either the start or the end of the block have to be on the specified line
        return sourceFile->isOffsetOnLine(offset, lineIndex) || sourceFile->isOffsetOnLine(offset + tok.charsCount, lineIndex);
    }
    else
    {
        // Has to be exactly on the specified line
        return sourceFile->isOffsetOnLine(offset, lineIndex);
    }
}


SlangResult DocumentationContext::_findMarkup(const FindInfo& info, Location location, FoundMarkup& out)
{
    out.reset();

    const auto& toks = info.tokenList->m_tokens;
    const Index tokIndex = info.declTokenIndex;

    // The line index that the markoff starts from 
    Index lineIndex = -1; 
    // The token stream search direction, valid values are -1 and 1
    Index searchDirection = 0;
    // The starting token index
    Index startIndex = -1;

    if (location == Location::Before)
    {
        lineIndex = info.declLineIndex - 1;
        startIndex = tokIndex - 1;
        searchDirection = -1;
    }
    else
    {
        startIndex = _findAfterIndex(info, location);
        if (startIndex <= 0)
        {
            return SLANG_E_NOT_FOUND;
        }

        SourceView* sourceView = info.sourceView;
        SourceFile* sourceFile = sourceView->getSourceFile();

        // Let's relookup the line index
        const int offset = sourceView->getRange().getOffset(toks[startIndex - 1].loc);
        lineIndex = sourceFile->calcLineIndexFromOffset(offset);

        searchDirection = 1;
    }

    SLANG_ASSERT(searchDirection == -1 || searchDirection == 1);
    SLANG_ASSERT(startIndex > 0);
    if (lineIndex < 0)
    {
        return SLANG_E_NOT_FOUND;
    }
    
    // Get the type and flags
    const MarkupType type = findMarkupType(toks[startIndex]);
    const MarkupFlags flags = getFlags(type);

    const MarkupFlag::Enum requiredFlag = (location == Location::Before) ? MarkupFlag::Before : MarkupFlag::After;

    if ((flags & requiredFlag) == 0)
    {
        return SLANG_E_NOT_FOUND;
    }

    Index currentLineIndex = lineIndex;

    // The token still isn't accepted, unless it's on the expected line
    if (_isLineIndexOk(info.sourceView, type, location, toks[startIndex], currentLineIndex))
    {
        return SLANG_E_NOT_FOUND;
    }

    Index endIndex = startIndex + searchDirection;

    // If it's multiline, so look for the end index
    if (flags & MarkupFlag::IsMultiToken)
    {
        // TODO(JS):
        // We should probably do the work here to confirm that these are on separate lines, and ideally with the same indentation.
        // For now we don't bother
        while (endIndex >= 0 && endIndex < toks.getCount())
        {
            // Do we find a token of the right type?
            if (findMarkupType(toks[endIndex + searchDirection]) != type)
            {
                break;
            }

            // Is it on the right line?
            if (_isLineIndexOk(info.sourceView, type, location, toks[startIndex], currentLineIndex + searchDirection))
            {
                break;
            }

            currentLineIndex += searchDirection;
            endIndex += searchDirection;
        }
    }

    // Put start < end order
    if (endIndex < startIndex)
    {
        Swap(endIndex, startIndex);
    }

    // Okay we've found the markup
    out.type = type;
    out.location = location;
    out.range = IndexRange{ startIndex, endIndex };

    return SLANG_OK;
}

SlangResult DocumentationContext::_findFirstMarkup(const FindInfo& info, const Location* locs, Index locCount, FoundMarkup& out, Index& outIndex)
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

SlangResult DocumentationContext::_findMarkup(const FindInfo& info, const Location* locs, Index locCount, FoundMarkup& out)
{
    Index foundIndex;
    SLANG_RETURN_ON_FAIL(_findFirstMarkup(info, locs, locCount, out, foundIndex));

    // Lets see if the remaining ones match
    for (Index i = foundIndex + 1; i < locCount; ++i)
    {
        SlangResult res = _findMarkup(info, locs[i], out);
        if (SLANG_SUCCEEDED(res))
        {
            // TODO(JS): Warning found markup in another location
        }
    }

    return SLANG_OK;
}

SlangResult DocumentationContext::_findMarkup(const FindInfo& info, Decl* decl, FoundMarkup& out)
{
    if (auto paramDecl = as<ParamDecl>(decl))
    {
        Location locs[] = { Location::Before, Location::AfterParam };
        return _findMarkup(info, locs, SLANG_COUNT_OF(locs), out);
    }
    else if (auto callableDecl = as<CallableDecl>(decl))
    {
        // We allow it defined before
        return _findMarkup(info, Location::Before, out);
    }
    else if (as<VarDecl>(decl) || as<TypeDefDecl>(decl) || as<AssocTypeDecl>(decl))
    {
        Location locs[] = { Location::Before, Location::AfterSemicolon };
        return _findMarkup(info, locs, SLANG_COUNT_OF(locs), out);
    }
    else
    {
        // We'll only allow before
        return _findMarkup(info, Location::Before, out);
    }
}

SlangResult DocumentationContext::extract(Documentation* doc, ModuleDecl* moduleDecl, SourceManager* sourceManager, DiagnosticSink* sink)
{
    m_doc = doc;
    m_moduleDecl = moduleDecl;
    m_sourceManager = sourceManager;
    m_sink = sink;

    _findDecls(moduleDecl);

    struct Entry
    {
        typedef Entry ThisType;

        bool operator<(const ThisType& rhs) const { return locOrOffset < rhs.locOrOffset; }

        Index viewIndex;                    ///< The view/file index this loc is found in
        SourceLoc::RawValue locOrOffset;    ///< Can be a loc or an offset into the file

        Decl* decl;                         ///< The decl
    };

    List<Entry> entries;

    {
        const Index count = m_decls.getCount();
        entries.setCount(count);

        for (Index i = 0; i < count; ++i)
        {
            Entry& entry = entries[i];
            auto decl = m_decls[i];
            entry.decl = decl;
            entry.viewIndex = -1;            //< We don't know what file/view it's in
            entry.locOrOffset = decl->loc.getRaw();
        }
    }

    // We hold one view per *SourceFile*
    List<SourceView*> views;

    // Sort them into loc order
    entries.sort([](Entry& a, Entry& b) { return a.locOrOffset < b.locOrOffset; });

    {
        SourceView* sourceView = nullptr;
        Index viewIndex = -1;

        for (auto& entry : entries)
        {
            const SourceLoc loc = SourceLoc::fromRaw(entry.locOrOffset);

            if (sourceView == nullptr || !sourceView->getRange().contains(loc))
            {
                // Find the new view
                sourceView = m_sourceManager->findSourceView(loc);
                SLANG_ASSERT(sourceView);

                // We want only one view per SourceFile
                SourceFile* sourceFile = sourceView->getSourceFile();

                // NOTE! The view found might be different than sourceView. 
                viewIndex = views.findFirstIndex([&](SourceView* currentView) -> bool { return currentView->getSourceFile() == sourceFile; });

                if (viewIndex < 0)
                {
                    viewIndex = views.getCount();
                    views.add(sourceView);
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
        entries.sort([](Entry& a, Entry& b) { return (a.viewIndex < b.viewIndex) || ((a.viewIndex == b.viewIndex) && a.locOrOffset < b.locOrOffset); });
    }

    {
        TokenList tokens;

        MemoryArena memoryArena;
        RootNamePool rootNamePool;
        NamePool namePool;
        namePool.setRootNamePool(&rootNamePool);

        Index viewIndex = -1;
        SourceView* sourceView = nullptr;

        for (auto& entry : entries)
        {
            if (viewIndex != entry.viewIndex)
            {
                viewIndex = entry.viewIndex;
                sourceView = views[viewIndex];

                // Make all memory free again
                memoryArena.reset();

                // Run the lexer
                Lexer lexer;
                lexer.initialize(sourceView, sink, &namePool, &memoryArena, kLexerFlag_TokenizeComments);

                // Lex everything
                tokens = lexer.lexAllTokens();
            }

            // Get the offset within the source file
            const uint32_t offset = entry.locOrOffset;

            // We need to get the loc in the source views space, so we look up appropriately in the list of tokens (which uses the views loc range)
            const SourceLoc loc = sourceView->getRange().getSourceLocFromOffset(offset);

            // Work out the line number
            SourceFile* sourceFile = sourceView->getSourceFile();
            const Index lineIndex = sourceFile->calcLineIndexFromOffset(int(offset));

            // Okay, lets find the token index with a binary chop
            Index tokenIndex = _findTokenIndex(loc, tokens.m_tokens.getBuffer(), tokens.m_tokens.getCount());
            if (tokenIndex >= 0 && lineIndex >= 0)
            {
                FindInfo findInfo;
                findInfo.declTokenIndex = tokenIndex;
                findInfo.declLineIndex = lineIndex;
                findInfo.tokenList = &tokens;
                findInfo.sourceView = sourceView;

                // Okay let's see if we extract some documentation then for this.
                FoundMarkup foundMarkup;
                SlangResult res = _findMarkup(findInfo, entry.decl, foundMarkup);

                if (SLANG_SUCCEEDED(res))
                {
                    // We need to extract
                    StringBuilder buf;
                    SLANG_RETURN_ON_FAIL(_extractMarkup(findInfo, foundMarkup, buf));

                    // Add to the documentation
                    Documentation::Entry& docEntry = m_doc->addEntry(entry.decl);
                    docEntry.m_markup = buf;
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

SlangResult Documentation::extract(ModuleDecl* moduleDecl, SourceManager* sourceManager, DiagnosticSink* sink)
{
    m_moduleDecl = moduleDecl;

    DocumentationContext context;
    return context.extract(this, moduleDecl, sourceManager, sink);
}

} // namespace Slang
