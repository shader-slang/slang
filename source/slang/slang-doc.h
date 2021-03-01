// slang-doc.h
#ifndef SLANG_DOC_H
#define SLANG_DOC_H

#include "../core/slang-basic.h"
#include "slang-ast-all.h"
#include "slang-ast-print.h"

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

        /// Given a module extracts all the associated markup.
    SlangResult extract(ModuleDecl* moduleDecl, SourceManager* sourceManager, DiagnosticSink* sink);

protected:

        /// The module this information was extracted from
    ModuleDecl* m_moduleDecl;
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

class SharedASTBuilder;

struct DocumentationUtil
{
    static SlangResult writeMarkdown(DocMarkup* markup, ASTBuilder* astBuilder, StringBuilder& out);
};

} // namespace Slang

#endif
