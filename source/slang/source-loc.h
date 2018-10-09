// source-loc.h
#ifndef SLANG_SOURCE_LOC_H_INCLUDED
#define SLANG_SOURCE_LOC_H_INCLUDED

#include "../core/basic.h"
#include "../core/slang-memory-arena.h"
#include "../core/slang-string-slice-pool.h"

#include "../../slang-com-ptr.h"
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
        /// True if the loc is in the range. Range is inclusive on begin to end.
    bool contains(SourceLoc loc) const { const auto rawLoc = loc.getRaw(); return rawLoc >= begin.getRaw() && rawLoc <= end.getRaw(); }
        /// Get the total size
    UInt getSize() const { return UInt(end.getRaw() - begin.getRaw()); }

        /// Get the offset of a loc in this range
    int getOffset(SourceLoc loc) const { SLANG_ASSERT(contains(loc)); return int(loc.getRaw() - begin.getRaw()); }

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

// A logical or physical storage object for a range of input code
// that has logically contiguous source locations.
class SourceFile : public RefObject
{
public:

        /// Returns the line break offsets (in bytes from start of content)
        /// Note that this is lazily evaluated - the line breaks are only calculated on the first request 
    const List<uint32_t>& getLineBreakOffsets();

        /// Calculate the line based on the offset 
    int calcLineIndexFromOffset(int offset);

        /// Calculate the offset for a line
    int calcColumnIndex(int line, int offset);

    // The logical file path to report for locations inside this span.
    String path;

    /// A blob that owns the storage for the file contents
    ComPtr<ISlangBlob> contentBlob;

    /// The actual contents of the file.
    UnownedStringSlice content;

    // The range of source locations that the span covers
    SourceRange sourceRange;

    protected:
    // In order to speed up lookup of line number information,
    // we will cache the starting offset of each line break in
    // the input file:
    List<uint32_t> m_lineBreakOffsets;
};

struct SourceManager;

enum class HumaneSourceLocType
{
    Normal,                 ///< Takes into account #line directives
    Original,               ///< Ignores #line directives - humane location as seen in the actual file
};

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

/* A SourceUnit maps to a single span of SourceLoc range and is equivalent to a single include or use of a source file. 
It is distinct from a SourceFile - because a SourceFile may be included multiple times, with different interpretations (depending 
on #defines for example).s
*/ 
class SourceUnit: public RefObject
{
    public:

    // Each entry represents some contiguous span of locations that
    // all map to the same logical file.
    struct Entry
    {
        SourceLoc m_startLoc;                       ///< Where does this entry begin?
        StringSlicePool::Handle m_pathHandle;        ///< What is the presumed path for this entry
        int32_t m_lineAdjust;                       ///< Adjustment to apply to source line numbers when printing presumed locations. Relative to the line number in the underlying file. 
    };

        /// Given a sourceLoc finds the entry associated with it. If returns -1 then no entry is 
        /// associated with this location, and therefore the location should be interpreted as an offset 
        /// into the underlying sourceFile.
    int findEntryIndex(SourceLoc sourceLoc) const;

        /// Add a line directive for this unit. The directiveLoc must of course be in this SourceUnit
        /// The path handle, must have been constructed on the SourceManager associated with the unit
        /// NOTE! Directives are assumed to be added IN ORDER during parsing such that every directiveLoc > previous 
    void addLineDirective(SourceLoc directiveLoc, StringSlicePool::Handle pathHandle, int line);

        /// Get the range that this unit applies to
    const SourceRange& getRange() const { return m_range; }
        /// Get the entries
    const List<Entry>& getEntries() const { return m_entries; }
        /// Get the source file holds the contents this 'unit' 
    SourceFile* getSourceFile() const { return m_sourceFile; }
        /// Get the source manager
    SourceManager* getSourceManager() const { return m_sourceManager; }

        /// Get the humane location 
    HumaneSourceLoc getHumaneLoc(HumaneSourceLocType type, SourceLoc loc);

        /// Ctor
    SourceUnit(SourceManager* sourceManager, SourceFile* sourceFile, SourceRange range):
        m_sourceManager(sourceManager),
        m_range(range),
        m_sourceFile(sourceFile)
    {
    }

    protected:
    
    SourceManager* m_sourceManager;     /// Get the manager this belongs to 
    SourceRange m_range;                ///< The range that this SourceUnit applies to
    RefPtr<SourceFile> m_sourceFile;    ///< The source file can hold the line breaks
    List<Entry> m_entries;              ///< An array entries describing how we should interpret a range, starting from the start location. 
};

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

        /// Allocate a new source unit from a file
    SourceUnit* newSourceUnit(SourceFile* sourceFile);


    // Get the source location that represents the spelling location corresponding to a location.
    SourceLoc getSpellingLoc(ExpandedSourceLoc const& loc);
    SourceLoc getSpellingLoc(SourceLoc const& loc);

        /// Find a unit by a source file location. If not found in this will look in the parent/
        /// Returns nullptr if not found
    SourceUnit* findSourceUnit(SourceLoc loc);

        /// Searches this manager, and then the parent to see if can find a match for path. 
        /// If not found returns nullptr.    
    SourceFile* findSourceFile(const String& path);

        /// Add a source file
    void addSourceFile(const String& path, SourceFile* sourceFile);

        /// Get the slice pool
    StringSlicePool& getStringSlicePool() { return m_slicePool; }

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

    protected:

    // All of the source units. These are held in increasing order of range, so can find by doing a binary chop.
    List<RefPtr<SourceUnit> > m_sourceUnits;                
    StringSlicePool m_slicePool;

    Dictionary<String, RefPtr<SourceFile> > m_sourceFiles;
};


} // namespace Slang

#endif
