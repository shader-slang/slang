// source-loc.cpp
#include "source-loc.h"

namespace Slang {

String ExpandedSourceLoc::getPath() const
{
    if(!sourceManager)
        return String();

    return sourceManager->sourceFiles[entryIndex].path;
}

String ExpandedSourceLoc::getSpellingPath() const
{
    if(!sourceManager)
        return String();

    return sourceManager->sourceFiles[entryIndex].sourceFile->path;
}

SourceFile* ExpandedSourceLoc::getSourceFile() const
{
    if(!sourceManager)
        return nullptr;

    return sourceManager->sourceFiles[entryIndex].sourceFile;
}

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

SourceFile* SourceManager::allocateSourceFile(
    String const&   path,
    String const&   content)
{
    UInt size = content.Length();

    SourceRange sourceRange = allocateSourceRange(size);

    SourceFile* sourceFile = new SourceFile();
    sourceFile->path = path;
    sourceFile->content = content;
    sourceFile->sourceRange = sourceRange;

    Entry entry;
    entry.sourceFile = sourceFile;
    entry.startLoc = sourceRange.begin;
    entry.path = path;

    sourceFiles.Add(entry);

    return sourceFile;
}

SourceLoc SourceManager::allocateSourceFileForLineDirective(
    SourceLoc const&    directiveLoc,
    String const&       path,
    UInt                line)
{
    // First, we need to find out what file we are being asked to remap
    ExpandedSourceLoc expandedDirectiveLoc = expandSourceLoc(getSpellingLoc(directiveLoc));
    HumaneSourceLoc humaneDirectiveLoc = getHumaneLoc(expandedDirectiveLoc);

    SourceFile* sourceFile = expandedDirectiveLoc.getSourceFile();
    if(!sourceFile)
        return SourceLoc();

    // We are going to be wasteful here and allocate a range of source locations
    // that can cover the entire input file. This will lead to a problem with
    // memory usage if we ever had a large input file that used many `#line` directives,
    // since our usage of ranges would be quadratic!

    // Count how many locations we'd need to reserve for a complete clone of the input
    UInt size = sourceFile->sourceRange.end.getRaw() - sourceFile->sourceRange.begin.getRaw();

    // Allocate a fresh range for our logically remapped file
    SourceRange sourceRange = allocateSourceRange(size);

    // Now fill in an entry that will point at the original source file,
    // but use our new range.
    Entry entry;
    entry.sourceFile = sourceFile;
    entry.startLoc = sourceRange.begin;
    entry.path = path;

    // We also need to make sure that any lookups for line numbers will
    // get corrected based on this files location.
    entry.lineAdjust = Int(line) - Int(humaneDirectiveLoc.line + 1);

    sourceFiles.Add(entry);

    return entry.startLoc;
}

static ExpandedSourceLoc expandSourceLoc(
    SourceManager*      inSourceManager,
    SourceLoc const&    loc)
{
    SourceManager* sourceManager = inSourceManager;

    ExpandedSourceLoc expanded;

    SourceLoc::RawValue rawValue = loc.getRaw();

    // Invalid location? -> invalid expanded location
    if(rawValue == 0)
        return expanded;

    // Past the end of what we can handle? -> invalid
    if(rawValue >= sourceManager->nextLoc.getRaw())
        return expanded;

    // Maybe the location came from a parent source manager
    while( rawValue < sourceManager->startLoc.getRaw()
        && sourceManager->parent)
    {
        sourceManager = sourceManager->parent;
    }

    SLANG_ASSERT(sourceManager->sourceFiles.Count() > 0);

    UInt lo = 0;
    UInt hi = sourceManager->sourceFiles.Count();

    while( lo+1 < hi )
    {
        UInt mid = lo + (hi - lo) / 2;

        SourceManager::Entry const& midEntry = sourceManager->sourceFiles[mid];
        SourceLoc::RawValue midValue = midEntry.startLoc.getRaw();

        if( midValue <= rawValue )
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

    // `lo` should now point at the entry we want
    UInt entryIndex = lo;

    expanded.setRaw(loc.getRaw());
    expanded.sourceManager = sourceManager;
    expanded.entryIndex = entryIndex;

    return expanded;


}

ExpandedSourceLoc SourceManager::expandSourceLoc(SourceLoc const& loc)
{
    return Slang::expandSourceLoc(this, loc);
}

HumaneSourceLoc SourceManager::getHumaneLoc(ExpandedSourceLoc const& loc)
{
    // First check if this location maps to an actual file.
    SourceFile* sourceFile = loc.getSourceFile();
    if(!sourceFile)
        return HumaneSourceLoc();

    auto& entry = sourceFiles[loc.entryIndex];
    UInt offset = loc.getRaw() - entry.startLoc.getRaw();

    // We now have a raw input file that we can search for line breaks.
    // We obviously don't want to do a linear scan over and over, so we will
    // cache an array of line break locations in the file.
    auto& lineBreakOffsets = sourceFile->lineBreakOffsets;
    if( lineBreakOffsets.Count() == 0 )
    {
        char const* begin = sourceFile->content.begin();
        char const* end = sourceFile->content.end();

        char const* cursor = begin;

        // Treat the beginning of the file as a line break
        lineBreakOffsets.Add(0);

        while( cursor != end )
        {
            int c = *cursor++;
            switch( c )
            {
            case '\r': case '\n':
                {
                    // When we see a line-break character we need
                    // to record the line break, but we also need
                    // to deal with the annoying issue of encodings,
                    // where a multi-byte sequence might encode
                    // the line break.

                    int d = *cursor;
                    if( (c^d) == ('\r' ^ '\n'))
                        cursor++;

                    lineBreakOffsets.Add(cursor - begin);
                }
                break;

            default:
                break;
            }
        }

        // Note taht we do *not* treat the end of the file as a line
        // break, because otherwise we would report errors like
        // "end of file inside string literal" with a line number
        // that points at a line that doesn't exist.
    }

    // At this point we can assume the `lineBreakOffsets` array has been filled in.
    // We will use a binary search to find the line index that contains our
    // chosen offset.
    UInt lo = 0;
    UInt hi = lineBreakOffsets.Count();

    while( lo+1 < hi )
    {
        UInt mid = lo + (hi - lo)/2;

        UInt midOffset = lineBreakOffsets[mid];
        if( midOffset <= offset )
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }

    UInt lineIndex = lo;
    UInt byteIndexInLine = offset - lineBreakOffsets[lineIndex];

    // Apply adjustment to the line number
    lineIndex = lineIndex + entry.lineAdjust;

    // TODO: we should really translate the byte index in the line
    // to deal with:
    //
    // - Non-ASCII characters, while might consume multiple bytes
    //
    // - Tab characters, which should really adjust how we report
    //   columns (although how are we supposed to know the setting
    //   that an IDE expects us to use when reporting locations?)

    HumaneSourceLoc humaneLoc;
    humaneLoc.path = entry.path;
    humaneLoc.line = lineIndex + 1;
    humaneLoc.column = byteIndexInLine + 1;

    return humaneLoc;
}

HumaneSourceLoc SourceManager::getHumaneLoc(SourceLoc const& loc)
{
    return getHumaneLoc(expandSourceLoc(loc));

}

SourceLoc SourceManager::getSpellingLoc(ExpandedSourceLoc const& loc)
{
    // First check if this location maps to some raw source file,
    // so that a "spelling" is even possible
    SourceFile* sourceFile = loc.getSourceFile();
    if(!sourceFile)
        return loc;

    // If we mapped to a source file, then the location must represent
    // some offset from an entry in our array.
    auto& entry = sourceFiles[loc.entryIndex];

    // We extract the offset of the location from the start of the entry
    SourceLoc::RawValue offsetFromStart = loc.getRaw() - entry.startLoc.getRaw();

    // And instead apply that offset to the spelling location of the file start
    SourceLoc result = sourceFile->sourceRange.begin + offsetFromStart;

    return result;
}

SourceLoc SourceManager::getSpellingLoc(SourceLoc const& loc)
{
    return getSpellingLoc(expandSourceLoc(loc));
}

} // namespace Slang
