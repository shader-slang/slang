// slang-doc.h
#ifndef SLANG_DOC_H
#define SLANG_DOC_H

#include "../core/slang-basic.h"
#include "slang-ast-all.h"

namespace Slang {

class Documentation : public RefObject
{
public:
    struct Entry
    {
        NodeBase* m_node;           ///< The node this documentation is associated with
        String m_markup;            ///< The raw contents of of markup associated with the decoration
        String m_contents;          ///< The documentation contents
    };

        /// Adds an entry, returns the reference to pre-existing node if there is one
    Entry& addEntry(NodeBase* base);
        /// Get's an entry for a node. Returns nullptr if there is no information.
    Entry* getEntry(NodeBase* base);

        /// Given a root node extracts all the associated documentation
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
SLANG_INLINE Documentation::Entry& Documentation::addEntry(NodeBase* base)
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
SLANG_INLINE Documentation::Entry* Documentation::getEntry(NodeBase* base)
{
    Index* indexPtr = m_entryMap.TryGetValue(base);
    return (indexPtr) ? &m_entries[*indexPtr] : nullptr;
}

} // namespace Slang

#endif
