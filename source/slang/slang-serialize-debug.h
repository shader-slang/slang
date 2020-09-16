// slang-serialize-debug.h
#ifndef SLANG_SERIALIZE_DEBUG_H
#define SLANG_SERIALIZE_DEBUG_H

#include "../core/slang-riff.h"
#include "../core/slang-string-slice-pool.h"
#include "../core/slang-array-view.h"

#include "slang-serialize-types.h"

#include "slang-name.h"
#include "slang-source-loc.h"

namespace Slang {

class DebugSerialData
{
public:
    typedef DebugSerialData ThisType;

    typedef uint32_t SourceLoc;
    typedef SerialStringData::StringIndex StringIndex;

    // The list that contains all the subsequent modules
    static const FourCC kDebugFourCc = SLANG_FOUR_CC('S', 'd', 'e', 'b');

    static const FourCC kDebugStringFourCc = SLANG_FOUR_CC('S', 'd', 's', 't');
    static const FourCC kDebugLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'l', 'n');
    static const FourCC kDebugAdjustedLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'a', 'l');
    static const FourCC kDebugSourceInfoFourCc = SLANG_FOUR_CC('S', 'd', 's', 'o');

    struct SourceRange
    {
        typedef SourceRange ThisType;

        bool operator==(const ThisType& rhs) const { return m_start == rhs.m_start && m_end == rhs.m_end; }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        Slang::SourceLoc getSourceLoc(SourceLoc loc, SourceView* view) const
        {
            return (loc && contains(loc)) ? Slang::SourceLoc::fromRaw((loc - m_start) + SourceLoc(view->getRange().begin.getRaw())) : Slang::SourceLoc();
        }

        SourceLoc getCount() const { return m_end - m_start; }

        bool contains(SourceLoc loc) const { return loc >= m_start && loc <= m_end; }

            /// Set up a range that can't occur in practice
        static SourceRange getInvalid() { return SourceRange{ ~SourceLoc(0), ~SourceLoc(0) }; }

        SourceLoc m_start;              ///< The offset to the source
        SourceLoc m_end;                ///< The number of bytes in the source
    };

    struct DebugSourceInfo
    {
        typedef DebugSourceInfo ThisType;

        bool operator==(const ThisType& rhs) const
        {
            return m_pathIndex == rhs.m_pathIndex &&
                m_range == rhs.m_range && 
                m_numLineInfos == rhs.m_numLineInfos &&
                m_lineInfosStartIndex == rhs.m_lineInfosStartIndex &&
                m_numLineInfos == rhs.m_numLineInfos;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        StringIndex m_pathIndex;                 ///< Index to the string table

        SourceRange m_range;                    ///< The range of locations

        uint32_t m_numLines;                    ///< Total number of lines in source file

        uint32_t m_lineInfosStartIndex;         ///< Index into m_debugLineInfos
        uint32_t m_numLineInfos;                ///< The number of line infos

        uint32_t m_adjustedLineInfosStartIndex; ///< Adjusted start index
        uint32_t m_numAdjustedLineInfos;        ///< The number of line infos
    };

    struct DebugLineInfo
    {
        typedef DebugLineInfo ThisType;
        bool operator<(const ThisType& rhs) const { return m_lineStartOffset < rhs.m_lineStartOffset; }
        bool operator==(const ThisType& rhs) const
        {
            return m_lineStartOffset == rhs.m_lineStartOffset &&
                m_lineIndex == rhs.m_lineIndex;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        uint32_t m_lineStartOffset;               ///< The offset into the source file 
        uint32_t m_lineIndex;                     ///< Original line index
    };

    struct DebugAdjustedLineInfo
    {
        typedef DebugAdjustedLineInfo ThisType;
        bool operator==(const ThisType& rhs) const
        {
            return m_lineInfo == rhs.m_lineInfo &&
                m_adjustedLineIndex == rhs.m_adjustedLineIndex &&
                m_pathStringIndex == rhs.m_pathStringIndex;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }
        bool operator<(const ThisType& rhs) const { return m_lineInfo < rhs.m_lineInfo; }

        DebugLineInfo m_lineInfo;
        uint32_t m_adjustedLineIndex;             ///< The line index with the adjustment (if there is any). Is 0 if m_pathStringIndex is 0.
        StringIndex m_pathStringIndex;            ///< The path as an index
    };

    size_t calcSizeInBytes() const;
    void clear();

    Index findSourceInfoIndex(SourceLoc sourceLoc) const
    {
        const Index numInfos = m_debugSourceInfos.getCount();
        for (Index i = 0; i < numInfos; ++i)
        {
            if (m_debugSourceInfos[i].m_range.contains(sourceLoc))
            {
                return i;
            }
        }
        return -1;
    }

    bool operator==(const ThisType& rhs) const;

    Result writeContainer(SerialCompressionType moduleCompressionType, RiffContainer* container);
    Result readContainer(SerialCompressionType moduleCompressionType, RiffContainer::ListChunk* listChunk);

    // Data only set if we have debug information

    List<char> m_debugStringTable;              ///< String table for debug use only
    List<DebugLineInfo> m_debugLineInfos;        ///< Debug line information
    List<DebugAdjustedLineInfo> m_debugAdjustedLineInfos;        ///< Adjusted line infos
    List<DebugSourceInfo> m_debugSourceInfos;    ///< Debug source information
};

class DebugSerialReader : public RefObject
{
public:
    Index findViewIndex(DebugSerialData::SourceLoc loc);

    SourceLoc getSourceLoc(DebugSerialData::SourceLoc loc);

        /// Works out he fix to source locs.
        /// Returns the amount to be added to a source loc to get something in
    int calcFixSourceLoc(DebugSerialData::SourceLoc loc, DebugSerialData::SourceRange& outRange);

        /// Calc the loc
    static SourceLoc calcFixedLoc(DebugSerialData::SourceLoc loc, int fix, const DebugSerialData::SourceRange& range) { SLANG_ASSERT(range.contains(loc)); SLANG_UNUSED(range); return SourceLoc::fromRaw(SourceLoc::RawValue(loc + fix)); }

    SlangResult read(const DebugSerialData* serialData, SourceManager* sourceManager);

protected:
    struct View
    {
        DebugSerialData::SourceRange m_range;
        SourceView* m_sourceView;
    };

    List<View> m_views;             ///< All the views
    Index m_lastViewIndex = -1;     ///< Caches last lookup
};

/// Used to write serialized Debug information
class DebugSerialWriter : public RefObject
{
public:

    class DebugSourceFile : public RefObject
    {
    public:
        DebugSourceFile(SourceFile* sourceFile, SourceLoc::RawValue baseSourceLoc) :
            m_sourceFile(sourceFile),
            m_baseSourceLoc(baseSourceLoc)
        {
            // Need to know how many lines there are
            const List<uint32_t>& lineOffsets = sourceFile->getLineBreakOffsets();

            const auto numLineIndices = lineOffsets.getCount();

            // Set none as being used initially
            m_lineIndexUsed.setCount(numLineIndices);
            ::memset(m_lineIndexUsed.begin(), 0, numLineIndices * sizeof(uint8_t));
        }
        /// True if we have information on that line index
        bool hasLineIndex(int lineIndex) const { return m_lineIndexUsed[lineIndex] != 0; }
        void setHasLineIndex(int lineIndex) { m_lineIndexUsed[lineIndex] = 1; }

        SourceLoc::RawValue m_baseSourceLoc;            ///< The base source location

        SourceFile* m_sourceFile;                       ///< The source file
        List<uint8_t> m_lineIndexUsed;                  ///< Has 1 if the line is used
        List<uint32_t> m_usedLineIndices;               ///< Holds the lines that have been hit                 

        List<DebugSerialData::DebugLineInfo> m_lineInfos;   ///< The line infos
        List<DebugSerialData::DebugAdjustedLineInfo> m_adjustedLineInfos;  ///< The adjusted line infos
    };

        /// Add a source location. Returns the location that can be serialized.
    DebugSerialData::SourceLoc addSourceLoc(SourceLoc sourceLoc);

        /// Write into outDebugData
    void write(DebugSerialData* outDebugData);
    
    DebugSerialWriter(SourceManager* sourceManager):
        m_sourceManager(sourceManager),
        m_debugStringSlicePool(StringSlicePool::Style::Default),
        m_debugFreeSourceLoc(1)
    {
    }

    SourceManager* m_sourceManager;
    StringSlicePool m_debugStringSlicePool;             ///< Slices held just for debug usage
    SourceLoc::RawValue m_debugFreeSourceLoc;           /// Locations greater than this are free
    Dictionary<SourceFile*, RefPtr<DebugSourceFile> > m_debugSourceFileMap;
};

} // namespace Slang

#endif
