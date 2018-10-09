// source-loc.cpp
#include "source-loc.h"

#include "compiler.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceUnit !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

int SourceUnit::findEntryIndex(SourceLoc sourceLoc) const
{
    if (!m_range.contains(sourceLoc))
    {
        return -1;
    }

    const auto rawValue = sourceLoc.getRaw();

    int hi = int(m_entries.Count());    
    // If there are no entries, or it is in front of the first entry, then there is no associated entry
    if (hi == 0 || 
        m_entries[0].m_startLoc.getRaw() > sourceLoc.getRaw())
    {
        return -1;
    }

    int lo = 0;
    while (lo + 1 < hi)
    {
        int mid = (hi + lo) >> 1;

        const Entry& midEntry = m_entries[mid];
        SourceLoc::RawValue midValue = midEntry.m_startLoc.getRaw();

        if (midValue <= rawValue)
        {
            // The location we seek is at or after this entry
            lo = mid;
        }
        else
        {
            // The location we seek is before this entry
            hi = mid;
        }
    }

    return lo;
}

void SourceUnit::addLineDirective(SourceLoc directiveLoc, StringSlicePool::Handle pathHandle, int line)
{
    SLANG_ASSERT(pathHandle != StringSlicePool::Handle(0));
    SLANG_ASSERT(m_range.contains(directiveLoc));

    // Check that the directiveLoc values are always increasing
    SLANG_ASSERT(m_entries.Count() == 0 || (m_entries.Last().m_startLoc.getRaw() < directiveLoc.getRaw()));

    // Calculate the offset
    const int offset = m_range.getOffset(directiveLoc);
    
    // Get the line index in the original file
    const int lineIndex = m_sourceFile->calcLineIndexFromOffset(offset);

    Entry entry;
    entry.m_startLoc = directiveLoc;
    entry.m_pathHandle = pathHandle;
    
    // We also need to make sure that any lookups for line numbers will
    // get corrected based on this files location.
    // We assume the line number coming in is a line number, NOT an index so the correction needs + 1
    // There is an additional + 1 because we want the NEXT line to be 99 (ie +2 is correct 'fix')
    entry.m_lineAdjust = line - (lineIndex + 2);

    m_entries.Add(entry);
}

void SourceUnit::addLineDirective(SourceLoc directiveLoc, const String& path, int line)
{
    StringSlicePool::Handle pathHandle = m_sourceManager->getStringSlicePool().add(path.getUnownedSlice());
    return addLineDirective(directiveLoc, pathHandle, line);
}

void SourceUnit::addDefaultLineDirective(SourceLoc directiveLoc)
{
    SLANG_ASSERT(m_range.contains(directiveLoc));
    // Check that the directiveLoc values are always increasing
    SLANG_ASSERT(m_entries.Count() == 0 || (m_entries.Last().m_startLoc.getRaw() < directiveLoc.getRaw()));

    // Well if there are no entries, or the last one puts it in default case, then we don't need to add anything
    if (m_entries.Count() == 0 || (m_entries.Count() && m_entries.Last().isDefault()))
    {
        return;
    }

    Entry entry;
    entry.m_startLoc = directiveLoc;
    entry.m_lineAdjust = 0;                                 // No line adjustment... we are going back to default
    entry.m_pathHandle = StringSlicePool::Handle(0);        // Mark that there is no path, and that this is a 'default'

    SLANG_ASSERT(entry.isDefault());

    m_entries.Add(entry);
}

HumaneSourceLoc SourceUnit::getHumaneLoc(SourceLoc loc, SourceLocType type)
{
    const int offset = m_range.getOffset(loc);

    // We need the line index from the original source file
    const int lineIndex = m_sourceFile->calcLineIndexFromOffset(offset);
    
    // TODO: we should really translate the byte index in the line
    // to deal with:
    //
    // - Non-ASCII characters, while might consume multiple bytes
    //
    // - Tab characters, which should really adjust how we report
    //   columns (although how are we supposed to know the setting
    //   that an IDE expects us to use when reporting locations?)    
    const int columnIndex = m_sourceFile->calcColumnIndex(lineIndex, offset);

    HumaneSourceLoc humaneLoc;
    humaneLoc.column = columnIndex + 1;
    humaneLoc.line = lineIndex + 1;

    // Make up a default entry
    StringSlicePool::Handle pathHandle = StringSlicePool::Handle(0);

    // Only bother looking up the entry information if we want a 'Normal' lookup
    const int entryIndex = (type == SourceLocType::Normal) ? findEntryIndex(loc) : -1;
    if (entryIndex >= 0)
    {
        const Entry& entry = m_entries[entryIndex];
        // Adjust the line
        humaneLoc.line += entry.m_lineAdjust;
        // Get the pathHandle..
        pathHandle = entry.m_pathHandle;
    }

    // If there is no override path, then just the source files path
    if (pathHandle == StringSlicePool::Handle(0))
    {
        humaneLoc.path = m_sourceFile->path;
    }
    else
    {
        humaneLoc.path = m_sourceManager->getStringSlicePool().getSlice(pathHandle);
    }
    
    return humaneLoc;
}

String SourceUnit::getPath(SourceLoc loc, SourceLocType type)
{
    if (type == SourceLocType::Original)
    {
        return m_sourceFile->path;
    }

    const int entryIndex = findEntryIndex(loc);
    const StringSlicePool::Handle pathHandle = (entryIndex >= 0) ? m_entries[entryIndex].m_pathHandle : StringSlicePool::Handle(0);
   
    // If there is no override path, then just the source files path
    if (pathHandle == StringSlicePool::Handle(0))
    {
        return m_sourceFile->path;
    }
    else
    {
        return m_sourceManager->getStringSlicePool().getSlice(pathHandle);
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!! SourceFile !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

const List<uint32_t>& SourceFile::getLineBreakOffsets()
{
    // We now have a raw input file that we can search for line breaks.
    // We obviously don't want to do a linear scan over and over, so we will
    // cache an array of line break locations in the file.
    if (m_lineBreakOffsets.Count() == 0)
    {
        char const* begin = content.begin();
        char const* end = content.end();

        char const* cursor = begin;

        // Treat the beginning of the file as a line break
        m_lineBreakOffsets.Add(0);

        while (cursor != end)
        {
            int c = *cursor++;
            switch (c)
            {
                case '\r': case '\n':
                {
                    // When we see a line-break character we need
                    // to record the line break, but we also need
                    // to deal with the annoying issue of encodings,
                    // where a multi-byte sequence might encode
                    // the line break.

                    int d = *cursor;
                    if ((c^d) == ('\r' ^ '\n'))
                        cursor++;

                    m_lineBreakOffsets.Add(uint32_t(cursor - begin));
                    break;
                }
                default:
                    break;
            }
        }

        // Note that we do *not* treat the end of the file as a line
        // break, because otherwise we would report errors like
        // "end of file inside string literal" with a line number
        // that points at a line that doesn't exist.
    }

    return m_lineBreakOffsets;
}

int SourceFile::calcLineIndexFromOffset(int offset)
{
    SLANG_ASSERT(offset <= content.size());

    // Make sure we have the line break offsets
    const auto& lineBreakOffsets = getLineBreakOffsets();

    // At this point we can assume the `lineBreakOffsets` array has been filled in.
    // We will use a binary search to find the line index that contains our
    // chosen offset.
    int lo = 0;
    int hi = int(lineBreakOffsets.Count());

    while (lo + 1 < hi)
    {
        const int mid = (hi + lo) >> 1; 
        const uint32_t midOffset = lineBreakOffsets[mid];
        if (midOffset <= uint32_t(offset))
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }

    return lo;
}

int SourceFile::calcColumnIndex(int lineIndex, int offset)
{
    const auto& lineBreakOffsets = getLineBreakOffsets();
    return offset - lineBreakOffsets[lineIndex];   
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceManager !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void SourceManager::initialize(
    SourceManager*  p)
{
    parent = p;

    if( p )
    {
        // If we have a parent source manager, then we assume that all code at that level
        // has already been loaded, and it is safe to start our own source locations
        // right after those from the parent.
        //
        // TODO: more clever allocation in cases where that might not be reasonable
        startLoc = p->nextLoc;
    }
    else
    {
        // Location zero is reserved for an invalid location,
        // so we need to start reserving locations starting at 1.
        startLoc = SourceLoc::fromRaw(1);
    }

    nextLoc = startLoc;
}

SourceRange SourceManager::allocateSourceRange(UInt size)
{
    // TODO: consider using atomics here


    SourceLoc beginLoc  = nextLoc;
    SourceLoc endLoc    = beginLoc + size;

    // We need to be able to represent the location that is *at* the end of
    // the input source, so the next available location for a new file
    // must be placed one after the end of this one.

    nextLoc = endLoc + 1;

    return SourceRange(beginLoc, endLoc);
}

SourceFile* SourceManager::newSourceFile(
    String const&   path,
    ISlangBlob*     contentBlob)
{
    char const* contentBegin = (char const*) contentBlob->getBufferPointer();
    UInt contentSize = contentBlob->getBufferSize();
    char const* contentEnd = contentBegin + contentSize;

    SourceFile* sourceFile = new SourceFile();
    sourceFile->path = path;
    sourceFile->contentBlob = contentBlob;
    sourceFile->content = UnownedStringSlice(contentBegin, contentEnd);
 
    return sourceFile;
}

SourceFile* SourceManager::newSourceFile(
    String const&   path,
    String const&   content)
{
    ComPtr<ISlangBlob> contentBlob = createStringBlob(content);
    return newSourceFile(path, contentBlob);
}

SourceUnit* SourceManager::newSourceUnit(SourceFile* sourceFile)
{
    SourceRange range = allocateSourceRange(sourceFile->content.size());
    SourceUnit* sourceUnit = new SourceUnit(this, sourceFile, range);
    m_sourceUnits.Add(sourceUnit);

    return sourceUnit;
}

SourceUnit* SourceManager::findSourceUnit(SourceLoc loc)
{
    SourceLoc::RawValue rawLoc = loc.getRaw();

    int hi = int(m_sourceUnits.Count());

    if (hi == 0)
    {
        return nullptr;
    }

    int lo = 0;
    while (lo + 1 < hi)
    {
        int mid = (hi + lo) >> 1;

        SourceUnit* midEntry = m_sourceUnits[mid];
        if (midEntry->getRange().contains(loc))
        {
            return midEntry;
        }

        SourceLoc::RawValue midValue = midEntry->getRange().begin.getRaw();

        if (midValue <= rawLoc)
        {
            // The location we seek is at or after this entry
            lo = mid;
        }
        else
        {
            // The location we seek is before this entry
            hi = mid;
        }
    }

    // Check if low is a hit
    {
        SourceUnit* unit = m_sourceUnits[lo];
        if (unit->getRange().contains(loc))
        {
            return unit;
        }
    }

    // Check the parent if there is a parent
    return (parent) ? parent->findSourceUnit(loc) : nullptr;
}

SourceFile* SourceManager::findSourceFile(const String& path)
{
    RefPtr<SourceFile>* filePtr = m_sourceFiles.TryGetValue(path);
    if (filePtr)
    {
        return filePtr->Ptr();
    }
    return parent ? parent->findSourceFile(path) : nullptr;
}

void SourceManager::addSourceFile(const String& path, SourceFile* sourceFile)
{
    SLANG_ASSERT(!findSourceFile(path));
    m_sourceFiles.Add(path, sourceFile);
}

HumaneSourceLoc SourceManager::getHumaneLoc(SourceLoc loc, SourceLocType type)
{
    SourceUnit* sourceUnit = findSourceUnit(loc);
    if (sourceUnit)
    {
        return sourceUnit->getHumaneLoc(loc, type);
    }
    else
    {
        return HumaneSourceLoc();
    }
}

String SourceManager::getPath(SourceLoc loc, SourceLocType type)
{
    SourceUnit* sourceUnit = findSourceUnit(loc);
    if (sourceUnit)
    {
        return sourceUnit->getPath(loc, type);
    }
    else
    {
        return String("unknown");
    }
}


} // namespace Slang
