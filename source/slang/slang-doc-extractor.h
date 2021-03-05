// slang-doc.h
#ifndef SLANG_DOC_EXTRACTOR_H
#define SLANG_DOC_EXTRACTOR_H

#include "../core/slang-basic.h"
#include "slang-ast-all.h"

namespace Slang {

/* Holds the documentation markup that is associated with each node (typically a decl) from a module */
class DocMarkup : public RefObject
{
public:
    struct Entry
    {
        NodeBase* m_node;           ///< The node this documentation is associated with
        String m_markup;            ///< The raw contents of of markup associated with the decoration
    };

        /// Adds an entry, returns the reference to pre-existing node if there is one
    Entry& addEntry(NodeBase* base);
        /// Gets an entry for a node. Returns nullptr if there is no markup.
    Entry* getEntry(NodeBase* base);

        /// Get list of all of the entries in source order
    const List<Entry>& getEntries() const { return m_entries; }

protected:

        /// Map from AST nodes to documentation entries
    Dictionary<NodeBase*, Index> m_entryMap;
        /// All of the documentation entries in source order
    List<Entry> m_entries;
};

// ---------------------------------------------------------------------------
SLANG_INLINE DocMarkup::Entry& DocMarkup::addEntry(NodeBase* base)
{
    const Index count = m_entries.getCount();
    const Index index = m_entryMap.GetOrAddValue(base, count);

    if (index == count)
    {
        Entry entry;
        entry.m_node = base;
        m_entries.add(entry);
    }
    return m_entries[index];
}

// ---------------------------------------------------------------------------
SLANG_INLINE DocMarkup::Entry* DocMarkup::getEntry(NodeBase* base)
{
    Index* indexPtr = m_entryMap.TryGetValue(base);
    return (indexPtr) ? &m_entries[*indexPtr] : nullptr;
}

/* Extracts 'markup' from comments in Slang source core. The comments are extracted and associated in declarations. The association
is held in DocMarkup type. The comment style follows the doxygen style */
class DocMarkupExtractor
{
public:

    typedef uint32_t MarkupFlags;
    struct MarkupFlag
    {
        enum Enum : MarkupFlags
        {
            Before = 0x1,
            After = 0x2,
            IsMultiToken = 0x4,          ///< Can use more than one token
            IsBlock = 0x8,          ///< 
        };
    };

    // NOTE! Don't change order without fixing isBefore and isAfter
    enum class MarkupType
    {
        None,

        BlockBefore,        /// /**  */ or /*!  */.
        LineBangBefore,      /// //! Can be multiple lines
        LineSlashBefore,     /// /// Can be multiple lines

        BlockAfter,         /// /*!< */ or /**< */
        LineBangAfter,       /// //!< Can be multiple lines
        LineSlashAfter,      /// ///< Can be multiple lines
    };

    static bool isBefore(MarkupType type) { return Index(type) >= Index(MarkupType::BlockBefore) && Index(type) <= Index(MarkupType::LineSlashBefore); }
    static bool isAfter(MarkupType type) { return Index(type) >= Index(MarkupType::BlockAfter); }

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
        AfterEnumCase,                  ///< Can have a , or before }
        AfterGenericParam,              ///< Can have trailing , or > 
    };

    static bool isAfter(Location location) { return Index(location) >= Index(Location::AfterParam); }
    static bool isBefore(Location location) { return location == Location::Before; }

    struct FoundMarkup
    {
        void reset()
        {
            location = Location::None;
            type = MarkupType::None;
            range = IndexRange{ 0, 0 };
        }

        Location location = Location::None;
        MarkupType type = MarkupType::None;
        IndexRange range;
    };

    enum SearchStyle
    {
        None,               ///< Cannot be searched for
        EnumCase,           ///< An enum case
        Param,              ///< A parameter in a function/method
        Variable,           ///< A variable-like declaration
        Before,             ///< Only allows before
        Function,           ///< Function/method
        GenericParam,       ///< Generic parameter
    };

        /// An input search item
    struct SearchItemInput
    {
        SourceLoc sourceLoc;
        SearchStyle searchStyle;            ///< The search style when looking for an item
    };

        /// The items will be in source order
    struct SearchItemOutput
    {
        Index viewIndex;                    ///< Index into the array of views on the output
        Index inputIndex;                   ///< The index to this item in the input
        String text;                        ///< The found text
    };

    struct FindInfo
    {
        SourceView* sourceView;         ///< The source view the tokens were generated from
        TokenList* tokenList;           ///< The token list
        Index tokenIndex;               ///< The token index location (where searches start from)
        Index lineIndex;                ///< The line number for the decl
    };

        /// Extracts documentation from the nodes held in the module using the source manager. Found documentation is placed
        /// in outMarkup
    static SlangResult extract(ModuleDecl* moduleDecl, SourceManager* sourceManager, DiagnosticSink* sink, DocMarkup* outMarkup);

        /// Extracts 'markup' doc information for the specified input items
        /// The output is placed in out - with the items now in the source order *not* the order of the input items
        /// The inputIndex on the output holds the input item index
        /// The outViews holds the views specified in viewIndex in the output, which may be useful for determining where the documentation was placed in source
    SlangResult extract(const SearchItemInput* inputItems, Index inputCount, SourceManager* sourceManager, DiagnosticSink* sink, List<SourceView*>& outViews, List<SearchItemOutput>& out);

        /// Given a module finds all the decls, and places in outDecls
    static void findDecls(ModuleDecl* moduleDecl, List<Decl*>& outDecls);

        /// Given a decl determines the search style that is appropriate. Returns None if can't determine a suitable style
    static SearchStyle getSearchStyle(Decl* decl);
    
    static MarkupFlags getFlags(MarkupType type);
    static MarkupType findMarkupType(const Token& tok);
    static UnownedStringSlice removeStart(MarkupType type, const UnownedStringSlice& comment);

protected:
    /// returns SLANG_E_NOT_FOUND if not found, SLANG_OK on success else an error
    SlangResult _findMarkup(const FindInfo& info, Location location, FoundMarkup& out);

    /// Locations are processed in order, and the first successful used. If found in another location will issue a warning.
    /// returns SLANG_E_NOT_FOUND if not found, SLANG_OK on success else an error
    SlangResult _findFirstMarkup(const FindInfo& info, const Location* locs, Index locCount, FoundMarkup& out, Index& outIndex);

    SlangResult _findMarkup(const FindInfo& info, const Location* locs, Index locCount, FoundMarkup& out);

    /// Given the decl, the token stream, and the decls tokenIndex, try to find some associated markup
    SlangResult _findMarkup(const FindInfo& info, SearchStyle searchStyle, FoundMarkup& out);

    /// Given a found markup location extracts the contents of the tokens into out
    SlangResult _extractMarkup(const FindInfo& info, const FoundMarkup& foundMarkup, StringBuilder& out);

    /// Given a location, try to find the first token index that could potentially be markup
    /// Will return -1 if not found
    Index _findStartIndex(const FindInfo& info, Location location);

    /// True if the tok is 'on' lineIndex. Interpretation of 'on' depends on the markup type.
    static bool _isTokenOnLineIndex(SourceView* sourceView, MarkupType type, const Token& tok, Index lineIndex);

    DiagnosticSink* m_sink;
};

} // namespace Slang

#endif
