// slang-source-loc.cpp
#include "slang-source-loc.h"

#include "../core/slang-string-util.h"
#include "../core/slang-string-escape-util.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceView !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

const String PathInfo::getMostUniqueIdentity() const
{
    switch (type)
    {
        case Type::Normal:      return uniqueIdentity;
        case Type::FoundPath:   
        case Type::FromString:
        {
            return foundPath;
        }
        default:                return "";
    }
}

bool PathInfo::operator==(const ThisType& rhs) const
{
    // They must be the same type
    if (type != rhs.type)
    {
        return false;
    }

    switch (type)
    {
        case Type::TokenPaste:
        case Type::TypeParse:
        case Type::Unknown:
        case Type::CommandLine:
        {
            return true;
        }
        case Type::Normal:
        {
            return foundPath == rhs.foundPath && uniqueIdentity == rhs.uniqueIdentity;
        }
        case Type::FromString:
        case Type::FoundPath:
        {
            // Only have a found path
            return foundPath == rhs.foundPath;
        }
        default: break;
    }

    return false;
}

void PathInfo::appendDisplayName(StringBuilder& out) const
{
    switch (type)
    {
        case Type::TokenPaste:      out << "[Token Paste]"; break;
        case Type::TypeParse:       out << "[Type Parse]"; break;
        case Type::Unknown:         out << "[Unknown]"; break;
        case Type::CommandLine:     out << "[Command Line]"; break;
        case Type::Normal:          
        case Type::FromString:
        case Type::FoundPath:
        {
            StringEscapeUtil::appendQuoted(StringEscapeUtil::getHandler(StringEscapeUtil::Style::Cpp), foundPath.getUnownedSlice(), out);    
            break;
        }
        default: break;
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

    Index hi = m_entries.getCount();    
    // If there are no entries, or it is in front of the first entry, then there is no associated entry
    if (hi == 0 || 
        m_entries[0].m_startLoc.getRaw() > sourceLoc.getRaw())
    {
        return -1;
    }

    Index lo = 0;
    while (lo + 1 < hi)
    {
        const Index mid = (hi + lo) >> 1;
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

    return int(lo);
}

void SourceView::addLineDirective(SourceLoc directiveLoc, StringSlicePool::Handle pathHandle, int line)
{
    SLANG_ASSERT(pathHandle != StringSlicePool::Handle(0));
    SLANG_ASSERT(m_range.contains(directiveLoc));

    // Check that the directiveLoc values are always increasing
    SLANG_ASSERT(m_entries.getCount() == 0 || (m_entries.getLast().m_startLoc.getRaw() < directiveLoc.getRaw()));

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

    m_entries.add(entry);
}

void SourceView::addLineDirective(SourceLoc directiveLoc, const String& path, int line)
{
    StringSlicePool::Handle pathHandle = getSourceManager()->getStringSlicePool().add(path.getUnownedSlice());
    return addLineDirective(directiveLoc, pathHandle, line);
}

void SourceView::addDefaultLineDirective(SourceLoc directiveLoc)
{
    SLANG_ASSERT(m_range.contains(directiveLoc));
    // Check that the directiveLoc values are always increasing
    SLANG_ASSERT(m_entries.getCount() == 0 || (m_entries.getLast().m_startLoc.getRaw() < directiveLoc.getRaw()));

    // Well if there are no entries, or the last one puts it in default case, then we don't need to add anything
    if (m_entries.getCount() == 0 || (m_entries.getCount() && m_entries.getLast().isDefault()))
    {
        return;
    }

    Entry entry;
    entry.m_startLoc = directiveLoc;
    entry.m_lineAdjust = 0;                                 // No line adjustment... we are going back to default
    entry.m_pathHandle = StringSlicePool::Handle(0);        // Mark that there is no path, and that this is a 'default'

    SLANG_ASSERT(entry.isDefault());

    m_entries.add(entry);
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

    humaneLoc.pathInfo = _getPathInfoFromHandle(pathHandle);
    return humaneLoc;
}

PathInfo SourceView::_getPathInfo() const
{
    if (m_viewPath.getLength())
    {
        PathInfo pathInfo(m_sourceFile->getPathInfo());
        pathInfo.foundPath = m_viewPath;
        return pathInfo;
    }
    else
    {
        return m_sourceFile->getPathInfo();
    }
}

PathInfo SourceView::_getPathInfoFromHandle(StringSlicePool::Handle pathHandle) const
{
    // If there is no override path, then just the source files path
    if (pathHandle == StringSlicePool::Handle(0))
    {
        return _getPathInfo();
    }
    else
    {
        return PathInfo::makePath(getSourceManager()->getStringSlicePool().getSlice(pathHandle));
    }
}

PathInfo SourceView::getPathInfo(SourceLoc loc, SourceLocType type)
{
    if (type == SourceLocType::Actual)
    {
        return _getPathInfo();
    }

    const int entryIndex = findEntryIndex(loc);
    return _getPathInfoFromHandle((entryIndex >= 0) ? m_entries[entryIndex].m_pathHandle : StringSlicePool::Handle(0));
}

/* !!!!!!!!!!!!!!!!!!!!!!! SourceFile !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void SourceFile::setLineBreakOffsets(const uint32_t* offsets, UInt numOffsets)
{
    m_lineBreakOffsets.clear();
    m_lineBreakOffsets.addRange(offsets, numOffsets);
}

const List<uint32_t>& SourceFile::getLineBreakOffsets()
{
    // We now have a raw input file that we can search for line breaks.
    // We obviously don't want to do a linear scan over and over, so we will
    // cache an array of line break locations in the file.
    if (m_lineBreakOffsets.getCount() == 0)
    {
        UnownedStringSlice content(getContent()), line;
        char const* contentBegin = content.begin();
        while (StringUtil::extractLine(content, line))
        {
            m_lineBreakOffsets.add(uint32_t(line.begin() - contentBegin));
        }
        // Note that we do *not* treat the end of the file as a line
        // break, because otherwise we would report errors like
        // "end of file inside string literal" with a line number
        // that points at a line that doesn't exist.
    }

    return m_lineBreakOffsets;
}

SourceFile::OffsetRange SourceFile::getOffsetRangeAtLineIndex(Index lineIndex)
{
    const List<uint32_t>& offsets = getLineBreakOffsets();
    const Index count = offsets.getCount();

    if (lineIndex >= count - 1)
    {
        // Work out the line start
        const uint32_t offsetEnd = uint32_t(getContentSize());
        const uint32_t offsetStart = (lineIndex >= count) ? offsetEnd : offsets[lineIndex];
        // The line is the span from start, to the end of the content
        return OffsetRange{ offsetStart, offsetEnd };
    }
    else
    {
        const uint32_t offsetStart = offsets[lineIndex];
        const uint32_t offsetEnd = offsets[lineIndex + 1];
        return OffsetRange { offsetStart, offsetEnd };
    }
}

UnownedStringSlice SourceFile::getLineAtIndex(Index lineIndex)
{
    const OffsetRange range = getOffsetRangeAtLineIndex(lineIndex);

    if (range.isValid() && hasContent())
    {
        const UnownedStringSlice content = getContent();
        SLANG_ASSERT(range.end <= uint32_t(content.getLength()));

        const char*const text = content.begin();
        return UnownedStringSlice(text + range.start, text + range.end);
    }

    return UnownedStringSlice();
}

UnownedStringSlice SourceFile::getLineContainingOffset(uint32_t offset)
{
    const Index lineIndex = calcLineIndexFromOffset(offset);
    return getLineAtIndex(lineIndex);
}

bool SourceFile::isOffsetOnLine(uint32_t offset, Index lineIndex)
{
    const OffsetRange range = getOffsetRangeAtLineIndex(lineIndex);
    return range.isValid() && range.containsInclusive(offset);
}

int SourceFile::calcLineIndexFromOffset(int offset)
{
    SLANG_ASSERT(UInt(offset) <= getContentSize());

    // Make sure we have the line break offsets
    const auto& lineBreakOffsets = getLineBreakOffsets();

    // At this point we can assume the `lineBreakOffsets` array has been filled in.
    // We will use a binary search to find the line index that contains our
    // chosen offset.
    Index lo = 0;
    Index hi = lineBreakOffsets.getCount();

    while (lo + 1 < hi)
    {
        const Index mid = (hi + lo) >> 1; 
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

    return int(lo);
}

int SourceFile::calcColumnIndex(int lineIndex, int offset)
{
    const auto& lineBreakOffsets = getLineBreakOffsets();
    return offset - lineBreakOffsets[lineIndex];   
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceFile !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void SourceFile::setContents(ISlangBlob* blob)
{
    const UInt contentSize = blob->getBufferSize();

    SLANG_ASSERT(contentSize == m_contentSize);

    char const* contentBegin = (char const*)blob->getBufferPointer();
    char const* contentEnd = contentBegin + contentSize;

    m_contentBlob = blob;
    m_content = UnownedStringSlice(contentBegin, contentEnd);
}

void SourceFile::setContents(const String& content)
{
    ComPtr<ISlangBlob> contentBlob = StringUtil::createStringBlob(content);
    setContents(contentBlob);
}

SourceFile::SourceFile(SourceManager* sourceManager, const PathInfo& pathInfo, size_t contentSize) :
    m_sourceManager(sourceManager),
    m_pathInfo(pathInfo),
    m_contentSize(contentSize)
{
}

SourceFile::~SourceFile()
{
}

String SourceFile::calcVerbosePath() const
{
    ISlangFileSystemExt* fileSystemExt = getSourceManager()->getFileSystemExt();

    if (fileSystemExt)
    {
        String canonicalPath;
        ComPtr<ISlangBlob> canonicalPathBlob;
        if (SLANG_SUCCEEDED(fileSystemExt->getCanonicalPath(m_pathInfo.foundPath.getBuffer(), canonicalPathBlob.writeRef())))
        {
            canonicalPath = StringUtil::getString(canonicalPathBlob);
        }
        if (canonicalPath.getLength() > 0)
        {
            return canonicalPath;
        }
    }

    return m_pathInfo.foundPath;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! SourceManager !!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void SourceManager::initialize(
    SourceManager*  p,
    ISlangFileSystemExt* fileSystemExt)
{
    m_fileSystemExt = fileSystemExt;

    m_parent = p;

    _resetLoc();
}

SourceManager::~SourceManager()
{
    _resetSource();
}

void SourceManager::_resetLoc()
{
    if (m_parent)
    {
        // If we have a parent source manager, then we assume that all code at that level
        // has already been loaded, and it is safe to start our own source locations
        // right after those from the parent.
        //
        // TODO: more clever allocation in cases where that might not be reasonable
        m_startLoc = m_parent->m_nextLoc;
    }
    else
    {
        // Location zero is reserved for an invalid location,
        // so we need to start reserving locations starting at 1.
        m_startLoc = SourceLoc::fromRaw(1);
    }

    m_nextLoc = m_startLoc;
}

void SourceManager::_resetSource()
{
    for (auto item : m_sourceViews)
    {
        delete item;
    }

    for (auto item : m_sourceFiles)
    {
        delete item;
    }

    m_sourceViews.clear();
    m_sourceFiles.clear();

    m_sourceFileMap.Clear();
}


void SourceManager::reset()
{
    _resetSource();
    _resetLoc();
}

UnownedStringSlice SourceManager::allocateStringSlice(const UnownedStringSlice& slice)
{
    const UInt numChars = slice.getLength();

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

SourceFile* SourceManager::createSourceFileWithSize(const PathInfo& pathInfo, size_t contentSize)
{
    SourceFile* sourceFile = new SourceFile(this, pathInfo, contentSize);
    m_sourceFiles.add(sourceFile);
    return sourceFile;
}

SourceFile* SourceManager::createSourceFileWithString(const PathInfo& pathInfo, const String& contents)
{
    SourceFile* sourceFile = new SourceFile(this, pathInfo, contents.getLength());
    m_sourceFiles.add(sourceFile);
    sourceFile->setContents(contents);
    return sourceFile;
}

SourceFile* SourceManager::createSourceFileWithBlob(const PathInfo& pathInfo, ISlangBlob* blob)
{
    SourceFile* sourceFile = new SourceFile(this, pathInfo, blob->getBufferSize());
    m_sourceFiles.add(sourceFile);
    sourceFile->setContents(blob);
    return sourceFile;
}

SourceView* SourceManager::createSourceView(SourceFile* sourceFile, const PathInfo* pathInfo, SourceLoc initiatingSourceLoc)
{
    SourceRange range = allocateSourceRange(sourceFile->getContentSize());

    SourceView* sourceView = nullptr;
    if (pathInfo &&
        (pathInfo->foundPath.getLength() && sourceFile->getPathInfo().foundPath != pathInfo->foundPath))
    {
        sourceView = new SourceView(sourceFile, range, &pathInfo->foundPath, initiatingSourceLoc);
    }
    else
    {
        sourceView = new SourceView(sourceFile, range, nullptr, initiatingSourceLoc);
    }

    m_sourceViews.add(sourceView);

    return sourceView;
}

SourceView* SourceManager::findSourceView(SourceLoc loc) const
{
    Index hi = m_sourceViews.getCount();
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
    Index lo = 0;
    while (lo + 1 < hi)
    {
        Index mid = (hi + lo) >> 1;

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

SourceFile* SourceManager::findSourceFile(const String& uniqueIdentity) const
{
    SourceFile*const* filePtr = m_sourceFileMap.TryGetValue(uniqueIdentity);
    return (filePtr) ? *filePtr : nullptr;
}

SourceFile* SourceManager::findSourceFileRecursively(const String& uniqueIdentity) const
{
    const SourceManager* manager = this;
    do 
    {
        SourceFile* sourceFile = manager->findSourceFile(uniqueIdentity);
        if (sourceFile)
        {
            return sourceFile;
        }
        manager = manager->m_parent;
    } while (manager);
    return nullptr;
}

SourceFile* SourceManager::findSourceFileByContentRecursively(const char* text)
{
    const SourceManager* manager = this;
    do
    {
        SourceFile* sourceFile = manager->findSourceFileByContent(text);
        if (sourceFile)
        {
            return sourceFile;
        }
        manager = manager->m_parent;
    } while (manager);
    return nullptr;
}

SourceFile* SourceManager::findSourceFileByContent(const char* text) const
{
    for (SourceFile* sourceFile : getSourceFiles())
    {
        auto content = sourceFile->getContent();

        if (text >= content.begin() && text <= content.end())
        {
            return sourceFile;
        }
    }
    return nullptr;
}

void SourceManager::addSourceFile(const String& uniqueIdentity, SourceFile* sourceFile)
{
    SLANG_ASSERT(!findSourceFileRecursively(uniqueIdentity));
    m_sourceFileMap.Add(uniqueIdentity, sourceFile);
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
