// source-loc.cpp
#include "source-loc.h"

#include "compiler.h"

#include "../core/slang-string-util.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceView !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

const String PathInfo::getMostUniquePath() const
{
    switch (type)
    {
        case Type::Normal:      return canonicalPath;
        case Type::FoundPath:   return foundPath;
        default:                return "";
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceView !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

int SourceView::findEntryIndex(SourceLoc sourceLoc) const
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
        const int mid = (hi + lo) >> 1;
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

void SourceView::addLineDirective(SourceLoc directiveLoc, StringSlicePool::Handle pathHandle, int line)
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
    // We assume the line number coming from the directive is a line number, NOT an index, so the correction needs + 1
    // There is an additional + 1 because we want the NEXT line - ie the line after the #line directive, to the specified value
    // Taking both into account means +2 is correct 'fix'
    entry.m_lineAdjust = line - (lineIndex + 2);

    m_entries.Add(entry);
}

void SourceView::addLineDirective(SourceLoc directiveLoc, const String& path, int line)
{
    StringSlicePool::Handle pathHandle = m_sourceManager->getStringSlicePool().add(path.getUnownedSlice());
    return addLineDirective(directiveLoc, pathHandle, line);
}

void SourceView::addDefaultLineDirective(SourceLoc directiveLoc)
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

HumaneSourceLoc SourceView::getHumaneLoc(SourceLoc loc, SourceLocType type)
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
    const int entryIndex = (type == SourceLocType::Nominal) ? findEntryIndex(loc) : -1;
    if (entryIndex >= 0)
    {
        const Entry& entry = m_entries[entryIndex];
        // Adjust the line
        humaneLoc.line += entry.m_lineAdjust;
        // Get the pathHandle..
        pathHandle = entry.m_pathHandle;
    }

    humaneLoc.pathInfo = _getPathInfo(pathHandle);
    return humaneLoc;
}

PathInfo SourceView::_getPathInfo(StringSlicePool::Handle pathHandle) const
{
    // If there is no override path, then just the source files path
    if (pathHandle == StringSlicePool::Handle(0))
    {
        return m_sourceFile->pathInfo;
    }
    else
    {
        // We don't have a full normal path (including 'canonical') so just go with FoundPath
        return PathInfo::makePath(m_sourceManager->getStringSlicePool().getSlice(pathHandle));
    }
}

PathInfo SourceView::getPathInfo(SourceLoc loc, SourceLocType type)
{
    if (type == SourceLocType::Actual)
    {
        return m_sourceFile->pathInfo;
    }

    const int entryIndex = findEntryIndex(loc);
    return _getPathInfo((entryIndex >= 0) ? m_entries[entryIndex].m_pathHandle : StringSlicePool::Handle(0));
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
    SLANG_ASSERT(UInt(offset) <= content.size());

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
    m_parent = p;

    if( p )
    {
        // If we have a parent source manager, then we assume that all code at that level
        // has already been loaded, and it is safe to start our own source locations
        // right after those from the parent.
        //
        // TODO: more clever allocation in cases where that might not be reasonable
        m_startLoc = p->m_nextLoc;
    }
    else
    {
        // Location zero is reserved for an invalid location,
        // so we need to start reserving locations starting at 1.
        m_startLoc = SourceLoc::fromRaw(1);
    }

    m_nextLoc = m_startLoc;
}

UnownedStringSlice SourceManager::allocateStringSlice(const UnownedStringSlice& slice)
{
    const UInt numChars = slice.size();

    char* dst = (char*)m_memoryArena.allocate(numChars);
    ::memcpy(dst, slice.begin(), numChars);

    return UnownedStringSlice(dst, numChars);
}

SourceRange SourceManager::allocateSourceRange(UInt size)
{
    // TODO: consider using atomics here


    SourceLoc beginLoc  = m_nextLoc;
    SourceLoc endLoc    = beginLoc + size;

    // We need to be able to represent the location that is *at* the end of
    // the input source, so the next available location for a new file
    // must be placed one after the end of this one.

    m_nextLoc = endLoc + 1;

    return SourceRange(beginLoc, endLoc);
}

SourceFile* SourceManager::createSourceFile(const PathInfo& pathInfo, ISlangBlob* contentBlob)
{
    char const* contentBegin = (char const*) contentBlob->getBufferPointer();
    UInt contentSize = contentBlob->getBufferSize();
    char const* contentEnd = contentBegin + contentSize;

    SourceFile* sourceFile = new SourceFile();
    sourceFile->pathInfo = pathInfo;
    sourceFile->contentBlob = contentBlob;
    sourceFile->content = UnownedStringSlice(contentBegin, contentEnd);
 
    return sourceFile;
}
 
SourceFile* SourceManager::createSourceFile(const PathInfo& pathInfo, const String& content)
{
    ComPtr<ISlangBlob> contentBlob = StringUtil::createStringBlob(content);
    return createSourceFile(pathInfo, contentBlob);
}

SourceView* SourceManager::createSourceView(SourceFile* sourceFile)
{
    SourceRange range = allocateSourceRange(sourceFile->content.size());
    SourceView* sourceView = new SourceView(this, sourceFile, range);
    m_sourceViews.Add(sourceView);

    return sourceView;
}

SourceView* SourceManager::findSourceView(SourceLoc loc) const
{
    int hi = int(m_sourceViews.Count());
    // It must be in the range of this manager and have associated views for it to possibly be a hit
    if (!getSourceRange().contains(loc) || hi == 0)
    {
        return nullptr;
    }

    // If we don't have very many, we may as well just linearly search
    if (hi <= 8)
    {
        for (int i = 0; i < hi; ++i)
        {
            SourceView* view = m_sourceViews[i];
            if (view->getRange().contains(loc))
            {
                return view;
            }
        }
        return nullptr;
    }

    const SourceLoc::RawValue rawLoc = loc.getRaw();

    // Binary chop to see if we can find the associated SourceUnit
    int lo = 0;
    while (lo + 1 < hi)
    {
        int mid = (hi + lo) >> 1;

        SourceView* midView = m_sourceViews[mid];
        if (midView->getRange().contains(loc))
        {
            return midView;
        }

        const SourceLoc::RawValue midValue = midView->getRange().begin.getRaw();
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

    // Check if low is actually a hit
    SourceView* view = m_sourceViews[lo];
    return (view->getRange().contains(loc)) ? view : nullptr;
}

SourceView* SourceManager::findSourceViewRecursively(SourceLoc loc) const
{
    // Start with this manager
    const SourceManager* manager = this;
    do 
    {
        SourceView* sourceView = manager->findSourceView(loc);
        // If we found a hit we are done
        if (sourceView)
        {
            return sourceView;
        }
        // Try the parent
        manager = manager->m_parent;
    }
    while (manager);
    // Didn't find it
    return nullptr;
}

SourceFile* SourceManager::findSourceFile(const String& canonicalPath) const
{
    RefPtr<SourceFile>* filePtr = m_sourceFiles.TryGetValue(canonicalPath);
    return (filePtr) ? filePtr->Ptr() : nullptr;
}

SourceFile* SourceManager::findSourceFileRecursively(const String& canonicalPath) const
{
    const SourceManager* manager = this;
    do 
    {
        SourceFile* sourceFile = manager->findSourceFile(canonicalPath);
        if (sourceFile)
        {
            return sourceFile;
        }
        manager = manager->m_parent;
    } while (manager);
    return nullptr;
}

void SourceManager::addSourceFile(const String& canonicalPath, SourceFile* sourceFile)
{
    SLANG_ASSERT(!findSourceFileRecursively(canonicalPath));
    m_sourceFiles.Add(canonicalPath, sourceFile);
}

HumaneSourceLoc SourceManager::getHumaneLoc(SourceLoc loc, SourceLocType type)
{
    SourceView* sourceView = findSourceViewRecursively(loc);
    if (sourceView)
    {
        return sourceView->getHumaneLoc(loc, type);
    }
    else
    {
        return HumaneSourceLoc();
    }
}

PathInfo SourceManager::getPathInfo(SourceLoc loc, SourceLocType type)
{
    SourceView* sourceView = findSourceViewRecursively(loc);
    if (sourceView)
    {
        return sourceView->getPathInfo(loc, type);
    }
    else
    {
        return PathInfo::makeUnknown();
    }
}

} // namespace Slang
