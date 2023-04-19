#include "slang-source-map.h"

namespace Slang {

void SourceMap::clear()
{
    const String empty;

    m_file = empty;
    m_sourceRoot = empty;

    m_sources.clear();

    m_names.clear();

    m_sourcesContent.clear();

    m_lineStarts.setCount(1);
    m_lineStarts[0] = 0;

    m_lineEntries.clear();

    m_slicePool.clear();
}

SourceMap::SourceMap(const ThisType& rhs) :
    m_file(rhs.m_file),
    m_sourceRoot(rhs.m_sourceRoot),
    m_sources(rhs.m_sources),
    m_sourcesContent(rhs.m_sourcesContent),
    m_names(rhs.m_names),
    m_lineStarts(rhs.m_lineStarts),
    m_lineEntries(rhs.m_lineEntries),
    m_slicePool(rhs.m_slicePool)
{
}

SourceMap::ThisType& SourceMap::operator=(const ThisType& rhs)
{
    if (this != &rhs)
    {
        m_file = rhs.m_file;
        m_sourceRoot = rhs.m_sourceRoot;
        m_sources = rhs.m_sources;
        m_names = rhs.m_names;
        m_sourcesContent = rhs.m_sourcesContent;
        m_lineStarts = rhs.m_lineStarts;
        m_lineEntries = rhs.m_lineEntries;
        m_slicePool = rhs.m_slicePool;
    }

    return *this;
}

void SourceMap::swapWith(ThisType& rhs)
{
    m_file.swapWith(rhs.m_file);
    m_sourceRoot.swapWith(rhs.m_sourceRoot);
    m_sources.swapWith(rhs.m_sources);
    m_names.swapWith(rhs.m_names);
    m_sourcesContent.swapWith(rhs.m_sourcesContent);
    m_lineStarts.swapWith(rhs.m_lineStarts);
    m_lineEntries.swapWith(rhs.m_lineEntries);
    m_slicePool.swapWith(rhs.m_slicePool);
}

void SourceMap::advanceToLine(Index nextLineIndex)
{
    const Count currentLineIndex = getGeneratedLineCount() - 1;

    SLANG_ASSERT(nextLineIndex >= currentLineIndex);
    
    if (nextLineIndex <= currentLineIndex)
    {
        return;
    }

    const auto lastEntryIndex = m_lineEntries.getCount();

    // For all the new entries they will need to point to the end 
    m_lineStarts.setCount(nextLineIndex + 1);

    Index* starts = m_lineStarts.getBuffer();
    for (Index i = currentLineIndex + 1; i < nextLineIndex + 1; ++i)
    {
        starts[i] = lastEntryIndex;
    }
}

Index SourceMap::getNameIndex(const UnownedStringSlice& slice)
{
    StringSlicePool::Handle handle;

    if (!m_slicePool.findOrAdd(slice, handle))
    {
        // We know it can't possibly be used, so must be new (!)

        m_names.add(handle);
        return m_names.getCount() - 1;
    }

    // Okay, could already be in the list
    const auto index = m_names.indexOf(handle);
    if (index >= 0)
    {
        return index;
    }

    m_names.add(handle);
    return m_names.getCount() - 1;
}

UnownedStringSlice SourceMap::getSourceFileName(Index sourceFileIndex) const
{
    return m_slicePool.getSlice(m_sources[sourceFileIndex]);
}

Index SourceMap::getSourceFileIndex(const UnownedStringSlice& slice)
{
    StringSlicePool::Handle handle;

    if (!m_slicePool.findOrAdd(slice, handle))
    {
        // We know it can't possibly be used, so must be new (!)

        m_sources.add(handle);
        return m_sources.getCount() - 1;
    }

    // Okay, could already be in the list
    const auto index = m_sources.indexOf(handle);
    if (index >= 0)
    {
        return index;
    }

    m_sources.add(handle);
    return m_sources.getCount() - 1;
}

Index SourceMap::findEntry(Index lineIndex, Index colIndex) const
{
    auto entries = getEntriesForLine(lineIndex);

    Index closestDist = 0x7fffffff;
    Index bestIndex = -1;

    const Count count = entries.getCount();
    for (Index i = 0; i < count; ++i)
    {
        const Entry& entry = entries[i];

        // We found an exact match
        if (entry.generatedColumn == colIndex)
        {
            bestIndex = i;
            break;
        }

        Index dist = entry.generatedColumn - colIndex;
        dist = (dist < 0) ? -dist : dist;

        if (dist < closestDist)
        {
            closestDist = dist;
            bestIndex = i;
        }
    }

    if (bestIndex < 0)
    {
        return bestIndex;
    }

    return m_lineStarts[lineIndex] + bestIndex;
}

} // namespace Slang
