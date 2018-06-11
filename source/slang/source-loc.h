// source-loc.h
#ifndef SLANG_SOURCE_LOC_H_INCLUDED
#define SLANG_SOURCE_LOC_H_INCLUDED

#include "../core/basic.h"
#include "../core/slang-com-ptr.h"

#include "../../slang.h"

namespace Slang {

class SourceLoc
{
public:
    typedef UInt RawValue;

private:
    RawValue raw;

public:
    SourceLoc()
        : raw(0)
    {}

    SourceLoc(
        SourceLoc const& loc)
        : raw(loc.raw)
    {}

    RawValue getRaw() const { return raw; }
    void setRaw(RawValue value) { raw = value; }

    static SourceLoc fromRaw(RawValue value)
    {
        SourceLoc result;
        result.setRaw(value);
        return result;
    }

    bool isValid() const
    {
        return raw != 0;
    }
};

inline SourceLoc operator+(SourceLoc loc, Int offset)
{
    return SourceLoc::fromRaw(loc.getRaw() + UInt(offset));
}

// A range of locations in the input source
struct SourceRange
{
    SourceRange()
    {}

    SourceRange(SourceLoc loc)
        : begin(loc)
        , end(loc)
    {}

    SourceRange(SourceLoc begin, SourceLoc end)
        : begin(begin)
        , end(end)
    {}

    SourceLoc begin;
    SourceLoc end;
};

// A logical or phyiscal storage object for a range of input code
// that has logically contiguous source locations.
class SourceFile : public RefObject
{
public:
    // The logical file path to report for locations inside this span.
    String path;

    /// A blob that owns the storage for the file contents
    ComPtr<ISlangBlob> contentBlob;

    /// The actual contents of the file.
    UnownedStringSlice content;

    // The range of source locations that the span covers
    SourceRange sourceRange;

    // In order to speed up lookup of line number information,
    // we will cache the starting offset of each line break in
    // the input file:
    List<UInt> lineBreakOffsets;
};

struct SourceManager;

// A source location in a format a human might like to see
struct HumaneSourceLoc
{
    String  path;
    Int     line = 0;
    Int     column = 0;

    String const& getPath() const { return path; }
    Int getLine() const { return line; }
    Int getColumn() const { return column; }
};

// A source location that has been expanded with the info
// needed to reconstruct a "humane" location if needed.
struct ExpandedSourceLoc : public SourceLoc
{
    // The source manager that owns this location
    SourceManager*  sourceManager = nullptr;

    // The entry index that is used to understand the location
    UInt            entryIndex = 0;

    // Get the nominal path for this location
    String getPath() const;

    // Get the actual file path where this location appears
    String getSpellingPath() const;

    // Get the original source file that holds this location
    SourceFile* getSourceFile() const;

    // Get a "humane" version of a source location
    HumaneSourceLoc getHumaneLoc();
};

HumaneSourceLoc getHumaneLoc(ExpandedSourceLoc const& loc);

struct SourceManager
{
    // Initialize a source manager, with an optional parent
    void initialize(
        SourceManager*  parent);

    SourceRange allocateSourceRange(UInt size);

    SourceFile* allocateSourceFile(
        String const&   path,
        ISlangBlob*     content);

    SourceFile* allocateSourceFile(
        String const&   path,
        String const&   content);

    SourceLoc allocateSourceFileForLineDirective(
        SourceLoc const&    directiveLoc,
        String const&       path,
        UInt                line);

    // Expand a source location to include more explicit info
    ExpandedSourceLoc expandSourceLoc(SourceLoc const& loc);

    // Get a "humane" version of a source location
    HumaneSourceLoc getHumaneLoc(SourceLoc const& loc);


    // Get the source location that represents the spelling location corresponding to a location.
    SourceLoc getSpellingLoc(ExpandedSourceLoc const& loc);
    SourceLoc getSpellingLoc(SourceLoc const& loc);

    // The first location available to this source manager
    // (may not be the first location of all, because we might
    // have a parent source manager)
    SourceLoc startLoc;

    // The "parent" source manager that owns locations ahead of `startLoc`
    SourceManager* parent = nullptr;

    // The location to be used by the next source file to be loaded
    SourceLoc nextLoc;

    // Each entry represents some contiguous span of locations that
    // all map to the same logical file.
    struct Entry
    {
        // Where does this entry begin?
        SourceLoc        startLoc;

        // The soure file that represents the actual data
        RefPtr<SourceFile>  sourceFile;

        // What is the presumed path for this entry
        String path;

        // Adjustment to apply to source line numbers when printing presumed locations
        Int lineAdjust = 0;
    };

    // An array of soure files we have loaded, ordered by
    // increasing starting location
    List<Entry> sourceFiles;
};


} // namespace Slang

#endif
