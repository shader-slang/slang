#ifndef SLANG_COMPILER_CORE_SOURCE_MAP_H
#define SLANG_COMPILER_CORE_SOURCE_MAP_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-string.h"
#include "../core/slang-list.h"
#include "../core/slang-rtti-info.h"

#include "../core/slang-string-slice-pool.h"

#include "slang-json-value.h"

namespace Slang {

struct SourceMap
{
    struct Entry
    {
        // Note! All column/line are zero indexed
        Index generatedColumn;          ///< The generated column
        Index sourceFileIndex;          ///< The index into the source name/contents
        Index sourceLine;               ///< The line number in the originating source
        Index sourceColumn;             ///< The column number in the originating source
        Index nameIndex;                ///< Name index
    };

        /// Decode from root into the source map
    SlangResult decode(JSONContainer* container, JSONValue root, DiagnosticSink* sink);

        /// Converts the source map contents into JSON
    SlangResult encode(JSONContainer* container, DiagnosticSink* sink, JSONValue& outValue);

        /// Get the total number of generated lines
    Count getGeneratedLineCount() const { return m_lineStarts.getCount() - 1; }
        /// Get the entries on the line
    SLANG_FORCE_INLINE ConstArrayView<Entry> getEntriesForLine(Index generatedLine) const;
    
        /// Clear the contents of the source map
    void clear();

        /// Ctor
    SourceMap():
        m_slicePool(StringSlicePool::Style::Default)
    {
        clear();
    }

    String m_file;
    String m_sourceRoot;

    List<StringSlicePool::Handle> m_sources;

        /// Storage for the contents. Can be unset null to indicate not set. 
    List<StringSlicePool::Handle> m_sourcesContent;
    
        /// The names
    List<StringSlicePool::Handle> m_names;

    List<Index> m_lineStarts;
    List<Entry> m_lineEntries;

    StringSlicePool m_slicePool;
};

// -------------------------------------------------------------
SLANG_FORCE_INLINE ConstArrayView<SourceMap::Entry> SourceMap::getEntriesForLine(Index generatedLine) const
{
    const Index start = m_lineStarts[generatedLine];
    const Index end = m_lineStarts[generatedLine + 1];
    
    const auto entries = m_lineEntries.begin();

    SLANG_ASSERT(start >= 0 && start < m_lineEntries.getCount());
    SLANG_ASSERT(end >= start && end >= 0 && end <= m_lineEntries.getCount());

    return ConstArrayView<SourceMap::Entry>(entries + start, end - start);
}


} // namespace Slang

#endif // SLANG_COMPILER_CORE_SOURCE_MAP_H
