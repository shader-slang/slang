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
            Multiline       = 0x4,
        };
    };

    enum class MarkupType
    {
        None,
        CStyleBefore,       /// /**  */ or /*!  */.
        CStyleAfter,        /// /*!< */ or /**< */

        CppBangBefore,      /// //! Can be multiple lines
        CppSlashBefore,     /// ///

        CppBangAfter,       /// //!< Can be multiple lines
        CppSlashAfter,      /// ///< 
    };

    struct IndexRange
    {
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

    struct SearchInfo
    {
        SourceView* sourceView;         ///< The source view the tokens were generated from
        TokenList* tokenList;           ///< The token list
        Int declTokenIndex;             ///< The token index location (where searches start from)
    };

    SlangResult extract(Documentation* doc, ModuleDecl* moduleDecl, SourceManager* sourceManager, DiagnosticSink* sink);

    static MarkupFlags getFlags(MarkupType type);
    static MarkupType findMarkupType(const Token& tok);

protected:
        /// returns SLANG_E_NOT_FOUND if not found, SLANG_OK on success else an error
    SlangResult _findMarkup(Location location, const TokenList& toks, Index tokIndex, FoundMarkup& out);

        /// Locations are processed in order, and the first successful used. If found in another location will issue a warning.
        /// returns SLANG_E_NOT_FOUND if not found, SLANG_OK on success else an error
    SlangResult _findFirstMarkup(const Location* locs, Index locCount, const TokenList& toks, Index tokIndex, FoundMarkup& out, Index& outIndex);

    SlangResult _findMarkup(const Location* locs, Index locCount, const TokenList& toks, Index tokIndex, FoundMarkup& out);

        /// Returns the index of the loc that succeeded. 
    Index _findMarkupIndex(const Location* locs, Index locCount, const TokenList& toks, Index tokIndex, FoundMarkup& out);

        /// Given the decl, the token stream, and the decls tokenIndex, try to find some associated markup
    SlangResult _findMarkup(Decl* decl, const TokenList& toks, Index tokenIndex, FoundMarkup& out);

        /// Given a found markup location extracts the contents of the tokens into out
    SlangResult _extractMarkup(const FoundMarkup& foundMarkup, const TokenList& toks, StringBuilder& out);

        /// Given a location, try to find the first token index that could potentially be markup
        /// Only works on 'after' locations, and will return -1 if not found
    Index _findAfterIndex(Location loc, const TokenList& toks, Index tokIndex);

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
        case MarkupType::CStyleBefore:      return MarkupFlag::Before; 
        case MarkupType::CStyleAfter:       return MarkupFlag::After;

        case MarkupType::CppBangBefore:     return MarkupFlag::Before | MarkupFlag::Multiline; 
        case MarkupType::CppSlashBefore:    return MarkupFlag::Before | MarkupFlag::Multiline; 

        case MarkupType::CppBangAfter:      return MarkupFlag::After | MarkupFlag::Multiline; 
        case MarkupType::CppSlashAfter:     return MarkupFlag::After | MarkupFlag::Multiline;
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
                return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::CStyleAfter : MarkupType::CStyleBefore;
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
                    return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::CppBangAfter : MarkupType::CppBangBefore;
                }
                else if (slice[2] == '/')
                {
                    return (slice.getLength() >= 4 && slice[3] == '<') ? MarkupType::CppSlashAfter : MarkupType::CppSlashBefore;
                }
            }
            break;
        }
        default: break;
    }
    return MarkupType::None;
}

SlangResult DocumentationContext::_extractMarkup(const FoundMarkup& foundMarkup, const TokenList& toks, StringBuilder& out)
{
    return SLANG_FAIL;
}

Index DocumentationContext::_findAfterIndex(Location location, const TokenList& toks, Index tokIndex)
{
    Index openParensCount = 0;

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

SlangResult DocumentationContext::_findMarkup(Location location, const TokenList& tokenList, Index tokIndex, FoundMarkup& out)
{
    out.reset();

    const auto& toks = tokenList.m_tokens;

    Index searchDirection = 0;
    Index startIndex = -1;
    if (location == Location::Before)
    {
        startIndex = tokIndex - 1;
        searchDirection = -1;
    }
    else
    {
        startIndex = _findAfterIndex(location, tokenList, tokIndex);
        searchDirection = 1;
    }
     
    if (startIndex < 0)
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

    Index endIndex = startIndex + searchDirection;

    // If it's multiline, so look for the end index
    if (flags & MarkupFlag::Multiline)
    {
        // TODO(JS):
        // We should probably do the work here to confirm that these are on separate lines, and ideally with the same indentation.
        // For now we don't bother
        while (endIndex >= 0 && endIndex < toks.getCount() && findMarkupType(toks[endIndex + searchDirection]) == type)
        {
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

SlangResult DocumentationContext::_findFirstMarkup(const Location* locs, Index locCount, const TokenList& toks, Index tokIndex, FoundMarkup& out, Index& outIndex)
{
    Index i = 0;
    for (; i < locCount; ++i)
    {
        SlangResult res = _findMarkup(locs[i], toks, tokIndex, out);
        if (SLANG_SUCCEEDED(res) || (SLANG_FAILED(res) && res != SLANG_E_NOT_FOUND))
        {
            outIndex = i;
            return res;
        }
    }
    return SLANG_E_NOT_FOUND;
}

SlangResult DocumentationContext::_findMarkup(const Location* locs, Index locCount, const TokenList& toks, Index tokIndex, FoundMarkup& out)
{
    Index foundIndex;
    SLANG_RETURN_ON_FAIL(_findFirstMarkup(locs, locCount, toks, tokIndex, out, foundIndex));

    // Lets see if the remaining ones match
    for (Index i = foundIndex + 1; i < locCount; ++i)
    {
        SlangResult res = _findMarkup(locs[i], toks, tokIndex, out);
        if (SLANG_SUCCEEDED(res))
        {
            // TODO(JS): Warning found markup in another location
        }
    }

    return SLANG_OK;
}

SlangResult DocumentationContext::_findMarkup(Decl* decl, const TokenList& toks, Index tokenIndex, FoundMarkup& out)
{
    if (auto paramDecl = as<ParamDecl>(decl))
    {
        Location locs[] = { Location::Before, Location::AfterParam };
        return _findMarkup(locs, SLANG_COUNT_OF(locs), toks, tokenIndex, out);
    }
    else if (auto callableDecl = as<CallableDecl>(decl))
    {
        // We allow it defined before
        return _findMarkup(Location::Before, toks, tokenIndex, out);
    }
    else if (as<VarDecl>(decl) || as<TypeDefDecl>(decl) || as<AssocTypeDecl>(decl))
    {
        Location locs[] = { Location::Before, Location::AfterSemicolon };
        return _findMarkup(locs, SLANG_COUNT_OF(locs), toks, tokenIndex, out);
    }
    else
    {
        // We'll only allow before
        return _findMarkup(Location::Before, toks, tokenIndex, out);
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

        bool operator<(const ThisType& rhs) const { return loc < rhs.loc; }

        Index fileIndex;                    ///< The file index this loc is found in
        SourceLoc::RawValue loc;            ///< Can be a loc or an offset into the file

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
            entry.fileIndex = -1;            //< We don't know what file it's in
            entry.loc = decl->loc.getRaw();
        }
    }

    struct FileAndView
    {
        SourceFile* file;           ///< The file
        SourceView* view;           ///< A view (any one will work) that maps to the file
    };

    List<FileAndView> fileAndViews;

    // Sort them into loc order
    entries.sort([](Entry& a, Entry& b) { return a.loc < b.loc; });

    {
        SourceView* sourceView = nullptr;
        Index fileIndex = -1;

        for (auto& entry : entries)
        {
            const SourceLoc loc = SourceLoc::fromRaw(entry.loc);

            if (sourceView == nullptr || !sourceView->getRange().contains(loc))
            {
                sourceView = m_sourceManager->findSourceView(loc);
                SLANG_ASSERT(sourceView);
                SourceFile* sourceFile = sourceView->getSourceFile();

                fileIndex = fileAndViews.findFirstIndex([&](const auto& fileAndView) -> bool { return fileAndView.file == sourceFile; });

                if (fileIndex < 0)
                {
                    fileIndex = fileAndViews.getCount();

                    FileAndView fileAndView{sourceFile, sourceView };
                    fileAndViews.add(fileAndView);
                }
            }

            SLANG_ASSERT(fileIndex >= 0);
            SLANG_ASSERT(sourceView && sourceView->getRange().contains(loc));

            // Set the file index
            entry.fileIndex = fileIndex;
            // Set the location within the file
            entry.loc = sourceView->getRange().getOffset(loc);
        }

        // Sort into file and then offset order
        entries.sort([](Entry& a, Entry& b) { return (a.fileIndex < b.fileIndex) || ((a.fileIndex == b.fileIndex) && a.loc < b.loc); });
    }

    {
        Index fileIndex = -1;
        TokenList tokens;

        MemoryArena memoryArena;
        RootNamePool rootNamePool;
        NamePool namePool;
        namePool.setRootNamePool(&rootNamePool);

        SourceView* sourceView = nullptr;

        for (auto& entry : entries)
        {
            if (fileIndex != entry.fileIndex)
            {
                const auto& fileAndView = fileAndViews[fileIndex];
                sourceView = fileAndView.view;

                // Make all memory free again
                memoryArena.reset();

                // Run the lexer
                Lexer lexer;
                lexer.initialize(sourceView, sink, &namePool, &memoryArena, kLexerFlag_TokenizeComments);

                fileIndex = entry.fileIndex;
                
                tokens = lexer.lexAllTokens();
            }

            // We need to get the loc in the source views space, so we look up appropriately in the list of tokens (which uses the views loc range)
            SourceLoc loc = sourceView->getRange().getSourceLocFromOffset(entry.loc);

            // Okay, lets find the token index with a binary chop

            Index tokenIndex = _findTokenIndex(loc, tokens.m_tokens.getBuffer(), tokens.m_tokens.getCount());
            if (tokenIndex >= 0)
            {
                // Okay let's see if we extract some documentation then for this.

                FoundMarkup foundMarkup;
                SlangResult res = _findMarkup(entry.decl, tokens, tokenIndex, foundMarkup);

                if (SLANG_SUCCEEDED(res))
                {
                    // We need to extract
                    StringBuilder buf;
                    SLANG_RETURN_ON_FAIL(_extractMarkup(foundMarkup, tokens, buf));

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
