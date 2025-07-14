// slang-serialize-source-loc.h
#ifndef SLANG_SERIALIZE_SOURCE_LOC_H
#define SLANG_SERIALIZE_SOURCE_LOC_H

#include "../compiler-core/slang-name.h"
#include "../compiler-core/slang-source-loc.h"
#include "../core/slang-array-view.h"
#include "../core/slang-riff.h"
#include "../core/slang-string-slice-pool.h"
#include "slang-serialize-types.h"
#include "slang-serialize.h"

namespace Slang
{

class SerialSourceLocData
{
public:
    typedef SerialSourceLocData ThisType;

    typedef uint32_t SourceLoc;
    typedef SerialStringData::StringIndex StringIndex;

    // The list that contains all the subsequent modules
    static const FourCC::RawValue kDebugFourCc = SLANG_FOUR_CC('S', 'd', 'e', 'b');

    static const FourCC::RawValue kDebugStringFourCc = SLANG_FOUR_CC('S', 'd', 's', 't');
    static const FourCC::RawValue kDebugLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'l', 'n');
    static const FourCC::RawValue kDebugAdjustedLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'a', 'l');
    static const FourCC::RawValue kDebugSourceInfoFourCc = SLANG_FOUR_CC('S', 'd', 's', 'o');

    struct SourceRange
    {
        typedef SourceRange ThisType;

        bool operator==(const ThisType& rhs) const
        {
            return m_start == rhs.m_start && m_end == rhs.m_end;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        Slang::SourceLoc getSourceLoc(SourceLoc loc, SourceView* view) const
        {
            return (loc && contains(loc))
                       ? Slang::SourceLoc::fromRaw(
                             (loc - m_start) + SourceLoc(view->getRange().begin.getRaw()))
                       : Slang::SourceLoc();
        }

        SourceLoc getCount() const { return m_end - m_start; }

        bool contains(SourceLoc loc) const { return loc >= m_start && loc <= m_end; }

        /// Set up a range that can't occur in practice
        static SourceRange getInvalid() { return SourceRange{~SourceLoc(0), ~SourceLoc(0)}; }

        SourceLoc m_start; ///< The offset to the source
        SourceLoc m_end;   ///< The number of bytes in the source
    };

    struct SourceInfo
    {
        typedef SourceInfo ThisType;

        bool operator==(const ThisType& rhs) const
        {
            return m_pathIndex == rhs.m_pathIndex && m_range == rhs.m_range &&
                   m_numLineInfos == rhs.m_numLineInfos &&
                   m_lineInfosStartIndex == rhs.m_lineInfosStartIndex &&
                   m_numLineInfos == rhs.m_numLineInfos;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        StringIndex m_pathIndex; ///< Index to the string table

        SourceRange m_range; ///< The range of locations

        uint32_t m_numLines; ///< Total number of lines in source file

        uint32_t m_lineInfosStartIndex; ///< Index into m_debugLineInfos
        uint32_t m_numLineInfos;        ///< The number of line infos

        uint32_t m_adjustedLineInfosStartIndex; ///< Adjusted start index
        uint32_t m_numAdjustedLineInfos;        ///< The number of line infos
    };

    struct LineInfo
    {
        typedef LineInfo ThisType;
        bool operator<(const ThisType& rhs) const
        {
            return m_lineStartOffset < rhs.m_lineStartOffset;
        }
        bool operator==(const ThisType& rhs) const
        {
            return m_lineStartOffset == rhs.m_lineStartOffset && m_lineIndex == rhs.m_lineIndex;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        uint32_t m_lineStartOffset; ///< The offset into the source file
        uint32_t m_lineIndex;       ///< Original line index
    };

    struct AdjustedLineInfo
    {
        typedef AdjustedLineInfo ThisType;
        bool operator==(const ThisType& rhs) const
        {
            return m_lineInfo == rhs.m_lineInfo && m_adjustedLineIndex == rhs.m_adjustedLineIndex &&
                   m_pathStringIndex == rhs.m_pathStringIndex;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }
        bool operator<(const ThisType& rhs) const { return m_lineInfo < rhs.m_lineInfo; }

        LineInfo m_lineInfo;
        uint32_t m_adjustedLineIndex;  ///< The line index with the adjustment (if there is any). Is
                                       ///< 0 if m_pathStringIndex is 0.
        StringIndex m_pathStringIndex; ///< The path as an index
    };

    size_t calcSizeInBytes() const;
    void clear();

    Index findSourceInfoIndex(SourceLoc sourceLoc) const
    {
        const Index numInfos = m_sourceInfos.getCount();
        for (Index i = 0; i < numInfos; ++i)
        {
            if (m_sourceInfos[i].m_range.contains(sourceLoc))
            {
                return i;
            }
        }
        return -1;
    }

    bool operator==(const ThisType& rhs) const;

    Result writeTo(RIFF::BuildCursor& cursor);
    Result readFrom(RIFF::ListChunk const* chunk);

    List<char> m_stringTable;                   ///< String table for debug use only
    List<LineInfo> m_lineInfos;                 ///< Line information
    List<AdjustedLineInfo> m_adjustedLineInfos; ///< Adjusted line infos
    List<SourceInfo> m_sourceInfos;             ///< Source infos
};

class SerialSourceLocReader : public RefObject
{
public:
    Index findViewIndex(SerialSourceLocData::SourceLoc loc);

    SourceLoc getSourceLoc(SerialSourceLocData::SourceLoc loc);

    /// Works out the amount to fix an input source loc to get a regular Slang::SourceLoc
    int calcFixSourceLoc(
        SerialSourceLocData::SourceLoc loc,
        SerialSourceLocData::SourceRange& outRange);

    /// Calc the loc
    static SourceLoc calcFixedLoc(
        SerialSourceLocData::SourceLoc loc,
        int fix,
        const SerialSourceLocData::SourceRange& range)
    {
        SLANG_ASSERT(range.contains(loc));
        SLANG_UNUSED(range);
        return SourceLoc::fromRaw(SourceLoc::RawValue(loc + fix));
    }

    SlangResult read(const SerialSourceLocData* serialData, SourceManager* sourceManager);

protected:
    struct View
    {
        SerialSourceLocData::SourceRange m_range;
        SourceView* m_sourceView;
    };

    List<View> m_views;         ///< All the views
    Index m_lastViewIndex = -1; ///< Caches last lookup
};

/// Used to write serialized SourceLoc information
class SerialSourceLocWriter : public RefObject
{
public:
    class Source : public RefObject
    {
    public:
        Source(SourceFile* sourceFile, SourceLoc::RawValue baseSourceLoc)
            : m_sourceFile(sourceFile), m_baseSourceLoc(baseSourceLoc)
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

        SourceLoc::RawValue m_baseSourceLoc; ///< The base source location

        SourceFile* m_sourceFile;         ///< The source file
        List<uint8_t> m_lineIndexUsed;    ///< Has 1 if the line is used
        List<uint32_t> m_usedLineIndices; ///< Holds the lines that have been hit

        List<SerialSourceLocData::LineInfo> m_lineInfos; ///< The line infos
        List<SerialSourceLocData::AdjustedLineInfo>
            m_adjustedLineInfos; ///< The adjusted line infos
    };

    /// Add a source location. Returns the location that can be serialized.
    SerialSourceLocData::SourceLoc addSourceLoc(SourceLoc sourceLoc);

    /// Write into outDebugData
    void write(SerialSourceLocData* outSourceLocData);

    SerialSourceLocWriter(SourceManager* sourceManager)
        : m_sourceManager(sourceManager)
        , m_stringSlicePool(StringSlicePool::Style::Default)
        , m_freeSourceLoc(1)
    {
    }

    SourceManager* m_sourceManager;
    StringSlicePool m_stringSlicePool;   ///< Slices held just for debug usage
    SourceLoc::RawValue m_freeSourceLoc; ///< Locations greater than this are free
    Dictionary<SourceFile*, RefPtr<Source>> m_sourceFileMap;
};

// A class a custom serialization context can inherit from to enable
// serializing SourceLoc
struct SourceLocSerialContext
{
    virtual SerialSourceLocReader* getSourceLocReader() { return nullptr; }
    virtual SerialSourceLocWriter* getSourceLocWriter() { return nullptr; }
};

template<typename S>
void serialize(S const& serializer, SourceLoc& value)
{
    static_assert(
        std::is_convertible_v<decltype(serializer.getContext()), SourceLocSerialContext*>,
        "getContext() must return a SourceLocSerialContext*");

    SourceLocSerialContext* context = serializer.getContext();

    // Writing of source location information can be disabled by
    // compiler options, and in that case the `_sourceLocWriter`
    // may be null.
    //
    // In order to handle that possibility, we serialize a `SourceLoc`
    // as an optional value, dependent on whether we have a
    // `_sourceLocWriter` that can be used.
    //
    SLANG_SCOPED_SERIALIZER_OPTIONAL(serializer);
    if (isWriting(serializer))
    {
        // The `SourceLoc` type is implemented under the hood as an
        // integer offset that can only be decoded using the specific
        // `SourceManager` that created it.
        //
        // The source location writer handles the task of translating
        // the under-the-hood representation to a single integer value
        // (represented as `SerialSourceLocData::SourceLoc`) that can
        // be decoded on the other side using other data that the
        // source location writer will write out as part of its own
        // representation (all of which goes into the dedicated debug
        // data chunk, distinct from the AST/IR module).
        //
        if (const auto sourceLocWriter = context->getSourceLocWriter())
        {
            SerialSourceLocData::SourceLoc rawValue = sourceLocWriter->addSourceLoc(value);
            serialize(serializer, rawValue);
        }
    }
    else
    {
        if (hasElements(serializer))
        {
            SerialSourceLocData::SourceLoc rawValue;
            serialize(serializer, rawValue);

            // Even if the serialized optional had a value, it is
            // possible that the debug-data chunk got stripped from
            // the compiled module file, in which case we wouldn't
            // have access to the data needed to decode it.
            //
            // In that case, the `_sourceLocReader` member would be
            // null, so we handle that possibility here.
            //
            if (const auto sourceLocReader = context->getSourceLocReader())
            {
                value = sourceLocReader->getSourceLoc(rawValue);
            }
        }
    }
}

// Now that we've seen the relevant serialization logic, it is clear that
// a `SourceLoc` gets fossilized the same way that an optional wrapping
// an integer (of type `SerialSourceLocData::SourceLoc`) would.
//
SLANG_DECLARE_FOSSILIZED_AS(SourceLoc, std::optional<SerialSourceLocData::SourceLoc>);

} // namespace Slang

#endif
