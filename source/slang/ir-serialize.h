// ir-serialize.h
#ifndef SLANG_IR_SERIALIZE_H_INCLUDED
#define SLANG_IR_SERIALIZE_H_INCLUDED

#include "../core/basic.h"
#include "../core/stream.h"

#include "../core/slang-object-scope-manager.h"

#include "ir.h"

// For TranslationUnitRequest
#include "compiler.h"

namespace Slang {

class StringRepresentationCache
{
    public:
    typedef StringSlicePool::Handle Handle;

    struct Entry
    {
        uint32_t m_startIndex;
        uint32_t m_numChars;
        RefObject* m_object;                ///< Could be nullptr, Name, or StringRepresentation. 
    };

        /// Get as a name
    Name* getName(Handle handle);
        /// Get as a string
    String getString(Handle handle);
        /// Get as string representation
    StringRepresentation* getStringRepresentation(Handle handle);
        /// Get as a string slice
    UnownedStringSlice getStringSlice(Handle handle) const;
        /// Get as a 0 terminated 'c style' string
    char* getCStr(Handle handle);

        /// Initialize a cache to use a string table, namePool and scopeManager
    void init(const List<char>* stringTable, NamePool* namePool, ObjectScopeManager* scopeManager);

        /// Ctor
    StringRepresentationCache(); 
    
    protected:
    ObjectScopeManager* m_scopeManager;
    NamePool* m_namePool;
    const List<char>* m_stringTable;
    List<Entry> m_entries;
};

struct SerialStringTableUtil
{
        /// Convert a pool into a string table
    static void encodeStringTable(const StringSlicePool& pool, List<char>& stringTable);
    static void encodeStringTable(const UnownedStringSlice* slices, size_t numSlices, List<char>& stringTable);
        /// Appends the decoded strings into slicesOut
    static void appendDecodedStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut);
        /// Decodes a string table (and does so such that the indices are compatible with StringSlicePool)
    static void decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut);

        /// Produces an index map, from slices to indices in pool
    static void calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMap);
};

// Pre-declare
class Name;

struct IRSerialData
{
    typedef IRSerialData ThisType;

    enum class InstIndex : uint32_t;
    enum class StringIndex : uint32_t;
    enum class ArrayIndex : uint32_t;

    enum class RawSourceLoc : SourceLoc::RawValue;          ///< This is just to copy over source loc data (ie not strictly serialize)
    enum class StringOffset : uint32_t;                     ///< Offset into the m_stringsBuffer
    
    typedef uint32_t SizeType;

    static const StringIndex kNullStringIndex = StringIndex(StringSlicePool::kNullHandle);
    static const StringIndex kEmptyStringIndex = StringIndex(StringSlicePool::kEmptyHandle);

    /// A run of instructions
    struct InstRun
    {
        typedef InstRun ThisType;
        SLANG_FORCE_INLINE bool operator==(const ThisType& rhs) const 
        {
            return m_parentIndex == rhs.m_parentIndex &&
                m_startInstIndex == rhs.m_startInstIndex &&
                m_numChildren == rhs.m_numChildren;
        }
        SLANG_FORCE_INLINE bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        InstIndex m_parentIndex;            ///< The parent instruction
        InstIndex m_startInstIndex;         ///< The index to the first instruction
        SizeType m_numChildren;                 ///< The number of children
    };

    struct SourceLocRun
    {
        typedef SourceLocRun ThisType;

        bool operator==(const ThisType& rhs) const { return m_sourceLoc == rhs.m_sourceLoc && m_startInstIndex == rhs.m_startInstIndex && m_numInst == rhs.m_numInst;  }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }
        bool operator<(const ThisType& rhs) const { return m_sourceLoc < rhs.m_sourceLoc;  }

        uint32_t m_sourceLoc;               ///< The source location
        InstIndex m_startInstIndex;         ///< The index to the first instruction
        SizeType m_numInst;                 ///< The number of children
    };

    struct PayloadInfo
    {
        uint8_t m_numOperands;
        uint8_t m_numStrings;
    };

    struct DebugSourceInfo
    {
        typedef DebugSourceInfo ThisType;

        bool operator==(const ThisType& rhs) const
        {
            return m_pathIndex == rhs.m_pathIndex &&
                m_startSourceLoc == rhs.m_startSourceLoc &&
                m_endSourceLoc == rhs.m_endSourceLoc &&
                m_numLineInfos == rhs.m_numLineInfos &&
                m_lineInfosStartIndex == rhs.m_lineInfosStartIndex &&
                m_numLineInfos == rhs.m_numLineInfos;
        }
        bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        bool isSourceLocInRange(uint32_t sourceLoc) const { return sourceLoc >= m_startSourceLoc && sourceLoc <= m_endSourceLoc;  }

        StringIndex m_pathIndex;                ///< Index to the string table
        uint32_t m_startSourceLoc;              ///< The offset to the source
        uint32_t m_endSourceLoc;                ///< The number of bytes in the source

        uint32_t m_numLines;                    ///< Total number of lines in source file

        uint32_t m_lineInfosStartIndex;         ///< Index into m_debugLineInfos
        uint32_t m_numLineInfos;                ///< The number of line infos

        uint32_t m_adjustedLineInfosStartIndex; ///< Adjusted start index
        uint32_t m_numAdjustedLineInfos;        ///< The number of line infos
    };

    struct DebugLineInfo
    {
        typedef DebugLineInfo ThisType;
        bool operator<(const ThisType& rhs) const { return m_lineStartOffset < rhs.m_lineStartOffset;  }
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
        bool operator<(const ThisType& rhs) const { return m_lineInfo < rhs.m_lineInfo;  }

        DebugLineInfo m_lineInfo;
        uint32_t m_adjustedLineIndex;             ///< The line index with the adjustment (if there is any). Is 0 if m_pathStringIndex is 0.
        StringIndex m_pathStringIndex;            ///< The path as an index
    };

    // Instruction...
    // We can store SourceLoc values separately. Just store per index information.
    // Parent information is stored in m_childRuns
    // Decoration information is stored in m_decorationRuns
    struct Inst
    {
        typedef Inst ThisType;
        enum
        {
            kMaxOperands = 2,           ///< Maximum number of operands that can be held in an instruction (otherwise held 'externally')
        };

        // NOTE! Can't change order or list without changing appropriate s_payloadInfos
        enum class PayloadType : uint8_t
        {
            // First 3 must be in this order so a cast from 0-2 is directly represented as number of operands 
            Empty,                          ///< Has no payload (or operands)
            Operand_1,                      ///< 1 Operand
            Operand_2,                      ///< 2 Operands

            OperandAndUInt32,               ///< 1 Operand and a single UInt32
            OperandExternal,                ///< Operands are held externally
            String_1,                       ///< 1 String
            String_2,                       ///< 2 Strings
            UInt32,                         ///< Holds an unsigned 32 bit integral (might represent a type)
            Float64,                    
            Int64,
            
            CountOf,
        };

            /// Get the number of operands
        SLANG_FORCE_INLINE int getNumOperands() const;

        bool operator==(const ThisType& rhs) const;
        
        SLANG_FORCE_INLINE bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        uint8_t m_op;                       ///< For now one of IROp 
        PayloadType m_payloadType;	 		///< The type of payload 
        uint16_t m_pad0;                    ///< Not currently used             

        InstIndex m_resultTypeIndex;	    //< 0 if has no type. The result type of this instruction

        struct ExternalOperandPayload
        {
            ArrayIndex m_arrayIndex;                        ///< Index into the m_externalOperands table
            SizeType m_size;                                ///< The amount of entries in that table
        };

        struct OperandAndUInt32
        {
            InstIndex m_operand;
            uint32_t m_uint32;
        };

        union Payload
        {
            double m_float64;
            int64_t m_int64;
            uint32_t m_uint32;                              ///< Unsigned integral value
            IRFloatingPointValue m_float;              ///< Floating point value
            IRIntegerValue m_int;                      ///< Integral value
            InstIndex m_operands[kMaxOperands];	            ///< For items that 2 or less operands it can use this.  
            StringIndex m_stringIndices[kMaxOperands];
            ExternalOperandPayload m_externalOperand;              ///< Operands are stored in an an index of an operand array 
            OperandAndUInt32 m_operandAndUInt32;
        };

        Payload m_payload;
    };
 
        /// Clear to initial state
    void clear();
        /// Get the operands of an instruction
    SLANG_FORCE_INLINE int getOperands(const Inst& inst, const InstIndex** operandsOut) const;

        /// ==
    bool operator==(const ThisType& rhs) const;
    SLANG_FORCE_INLINE bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }

        /// Calculate the amount of memory used by this IRSerialData
    size_t calcSizeInBytes() const;

    /// Ctor
    IRSerialData();
    
    List<Inst> m_insts;                         ///< The instructions

    List<InstRun> m_childRuns;                  ///< Holds the information about children that belong to an instruction

    List<InstIndex> m_externalOperands;         ///< Holds external operands (for instructions with more than kNumOperands)

    List<char> m_stringTable;                   ///< All strings. Indexed into by StringIndex

    List<RawSourceLoc> m_rawSourceLocs;         ///< A source location per instruction (saved without modification from IRInst)s

    // Data only set if we have debug information

    List<char> m_debugStringTable;              ///< String table for debug use only
    List<DebugLineInfo> m_debugLineInfos;        ///< Debug line information
    List<DebugAdjustedLineInfo> m_debugAdjustedLineInfos;        ///< Adjusted line infos
    List<DebugSourceInfo> m_debugSourceInfos;    ///< Debug source information
    List<SourceLocRun> m_debugSourceLocRuns;    ///< Runs of instructions that use a source loc

    static const PayloadInfo s_payloadInfos[int(Inst::PayloadType::CountOf)];
};

// --------------------------------------------------------------------------
SLANG_FORCE_INLINE int IRSerialData::Inst::getNumOperands() const
{
    return (m_payloadType == PayloadType::OperandExternal) ? m_payload.m_externalOperand.m_size : s_payloadInfos[int(m_payloadType)].m_numOperands;
}

// --------------------------------------------------------------------------
SLANG_FORCE_INLINE bool IRSerialData::Inst::operator==(const ThisType& rhs) const
{
    if (m_op == rhs.m_op && 
        m_payloadType == rhs.m_payloadType &&
        m_resultTypeIndex == rhs.m_resultTypeIndex)
    {
        switch (m_payloadType)
        {
            case PayloadType::Empty: 
            {
                return true;
            }
            case PayloadType::Operand_1:
            case PayloadType::String_1:
            case PayloadType::UInt32:
            {
                return m_payload.m_operands[0] == rhs.m_payload.m_operands[0];
            }
            case PayloadType::OperandAndUInt32:
            case PayloadType::OperandExternal:
            case PayloadType::Operand_2:
            case PayloadType::String_2:
            {
                return m_payload.m_operands[0] == rhs.m_payload.m_operands[0] &&
                       m_payload.m_operands[1] == rhs.m_payload.m_operands[1];
            } 
            case PayloadType::Float64:
            case PayloadType::Int64:
            {
                return m_payload.m_int64 == rhs.m_payload.m_int64;
            }
            default: break;
        }
    }

    return false;
}
// --------------------------------------------------------------------------
SLANG_FORCE_INLINE int IRSerialData::getOperands(const Inst& inst, const InstIndex** operandsOut) const
{
    if (inst.m_payloadType == Inst::PayloadType::OperandExternal)
    {
        *operandsOut = m_externalOperands.begin() + int(inst.m_payload.m_externalOperand.m_arrayIndex);
        return int(inst.m_payload.m_externalOperand.m_size);
    }
    else
    {
        *operandsOut = inst.m_payload.m_operands;
        return s_payloadInfos[int(inst.m_payloadType)].m_numOperands;
    }
}


#define SLANG_FOUR_CC(c0, c1, c2, c3) ((uint32_t(c0) << 0) | (uint32_t(c1) << 8) | (uint32_t(c2) << 16) | (uint32_t(c3) << 24)) 

#define SLANG_MAKE_COMPRESSED_FOUR_CC(fourCc) (((fourCc) & 0xffff00ff) | (uint32_t('c') << 8))

struct IRSerialBinary
{
    // http://fileformats.archiveteam.org/wiki/RIFF
    // http://www.fileformat.info/format/riff/egff.htm

    struct Chunk
    {
        uint32_t m_type;
        uint32_t m_size;
    };

    enum class CompressionType
    {
        None,
        VariableByteLite,
    };

    
    static const uint32_t kRiffFourCc = SLANG_FOUR_CC('R', 'I', 'F', 'F');
    static const uint32_t kSlangFourCc = SLANG_FOUR_CC('S', 'L', 'N', 'G');             ///< Holds all the slang specific chunks

    static const uint32_t kInstFourCc = SLANG_FOUR_CC('S', 'L', 'i', 'n');
    static const uint32_t kChildRunFourCc = SLANG_FOUR_CC('S', 'L', 'c', 'r');
    static const uint32_t kExternalOperandsFourCc = SLANG_FOUR_CC('S', 'L', 'e', 'o');

    static const uint32_t kCompressedInstFourCc = SLANG_MAKE_COMPRESSED_FOUR_CC(kInstFourCc);
    static const uint32_t kCompressedChildRunFourCc = SLANG_MAKE_COMPRESSED_FOUR_CC(kChildRunFourCc);
    static const uint32_t kCompressedExternalOperandsFourCc = SLANG_MAKE_COMPRESSED_FOUR_CC(kExternalOperandsFourCc);

    static const uint32_t kStringFourCc = SLANG_FOUR_CC('S', 'L', 's', 't');

    static const uint32_t kUInt32SourceLocFourCc = SLANG_FOUR_CC('S', 'r', 's', '4');

    static const uint32_t kDebugStringFourCc = SLANG_FOUR_CC('S', 'd', 's', 't');
    static const uint32_t kDebugLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'l', 'n');
    static const uint32_t kDebugAdjustedLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'a', 'l');
    static const uint32_t kDebugSourceInfoFourCc = SLANG_FOUR_CC('S', 'd', 's', 'o');
    static const uint32_t kDebugSourceLocRunFourCc = SLANG_FOUR_CC('S', 'd', 's', 'r');

    struct SlangHeader
    {
        Chunk m_chunk;
        uint32_t m_compressionType;         ///< Holds the compression type used (if used at all)
    };
    struct ArrayHeader
    {
        Chunk m_chunk;
        uint32_t m_numEntries;
    };
    struct CompressedArrayHeader
    {
        Chunk m_chunk;
        uint32_t m_numEntries;              ///< The number of entries
        uint32_t m_numCompressedEntries;    ///< The amount of compressed entries
    };
};


struct IRSerialWriter
{
    typedef IRSerialData Ser;
    typedef IRSerialBinary Bin;

    struct OptionFlag
    {
        typedef uint32_t Type;
        enum Enum: Type
        {
            RawSourceLocation       = 0x01,
            DebugInfo               = 0x02,
        };
    };
    typedef OptionFlag::Type OptionFlags;

    Result write(IRModule* module, SourceManager* sourceManager, OptionFlags options, IRSerialData* serialData);
  
    static Result writeStream(const IRSerialData& data, Bin::CompressionType compressionType, Stream* stream);

    /// Get an instruction index from an instruction
    Ser::InstIndex getInstIndex(IRInst* inst) const { return inst ? Ser::InstIndex(m_instMap[inst]) : Ser::InstIndex(0); }

        /// Get a slice from an index
    UnownedStringSlice getStringSlice(Ser::StringIndex index) const { return m_stringSlicePool.getSlice(StringSlicePool::Handle(index)); }
        /// Get index from string representations
    Ser::StringIndex getStringIndex(StringRepresentation* string) { return Ser::StringIndex(m_stringSlicePool.add(string)); }
    Ser::StringIndex getStringIndex(const UnownedStringSlice& slice) { return Ser::StringIndex(m_stringSlicePool.add(slice)); }
    Ser::StringIndex getStringIndex(Name* name) { return name ? getStringIndex(name->text) : Ser::kNullStringIndex; }
    Ser::StringIndex getStringIndex(const char* chars) { return Ser::StringIndex(m_stringSlicePool.add(chars)); }
    Ser::StringIndex getStringIndex(const String& string) { return Ser::StringIndex(m_stringSlicePool.add(string.getUnownedSlice())); }

    StringSlicePool& getStringPool() { return m_stringSlicePool;  }
    StringSlicePool& getDebugStringPool() { return m_debugStringSlicePool; }

    IRSerialWriter() :
        m_serialData(nullptr)
    {}

protected:
    class DebugSourceFile : public RefObject
    {
    public:
        DebugSourceFile(SourceFile* sourceFile, SourceLoc::RawValue baseSourceLoc):
            m_sourceFile(sourceFile),
            m_baseSourceLoc(baseSourceLoc)
        {
            // Need to know how many lines there are
            const List<uint32_t>& lineOffsets = sourceFile->getLineBreakOffsets();

            const auto numLineIndices = lineOffsets.Count();

            // Set none as being used initially
            m_lineIndexUsed.SetSize(numLineIndices);
            ::memset(m_lineIndexUsed.begin(), 0, numLineIndices * sizeof(uint8_t));
        }
            /// True if we have information on that line index
        bool hasLineIndex(int lineIndex) const { return m_lineIndexUsed[lineIndex] != 0; }
        void setHasLineIndex(int lineIndex) { m_lineIndexUsed[lineIndex] = 1; }

        SourceLoc::RawValue m_baseSourceLoc;            ///< The base source location

        SourceFile* m_sourceFile;                       ///< The source file
        List<uint8_t> m_lineIndexUsed;                  ///< Has 1 if the line is used
        List<uint32_t> m_usedLineIndices;               ///< Holds the lines that have been hit                 

        List<IRSerialData::DebugLineInfo> m_lineInfos;   ///< The line infos
        List<IRSerialData::DebugAdjustedLineInfo> m_adjustedLineInfos;  ///< The adjusted line infos
    };

    void _addInstruction(IRInst* inst);
    Result _calcDebugInfo();
        /// Returns the remapped sourceLoc, or 0 if sourceLoc couldn't be added
    void _addDebugSourceLocRun(SourceLoc sourceLoc, uint32_t startInstIndex, uint32_t numInst);

    List<IRInst*> m_insts;                              ///< Instructions in same order as stored in the 

    List<IRDecoration*> m_decorations;                  ///< Holds all decorations in order of the instructions as found
    List<IRInst*> m_instWithFirstDecoration;            ///< All decorations are held in this order after all the regular instructions

    Dictionary<IRInst*, Ser::InstIndex> m_instMap;      ///< Map an instruction to an instruction index

    StringSlicePool m_stringSlicePool;    
    IRSerialData* m_serialData;                         ///< Where the data is stored

    StringSlicePool m_debugStringSlicePool;             ///< Slices held just for debug usage

    SourceLoc::RawValue m_debugFreeSourceLoc;           /// Locations greater than this are free
    Dictionary<SourceFile*, RefPtr<DebugSourceFile> > m_debugSourceFileMap;
    
    SourceManager* m_sourceManager;                     ///< The source manager
};

struct IRSerialReader
{
    typedef IRSerialData Ser;
    typedef StringRepresentationCache::Handle StringHandle;

        /// Read a stream to fill in dataOut IRSerialData
    static Result readStream(Stream* stream, IRSerialData* dataOut);

        /// Read a module from serial data
    Result read(const IRSerialData& data, Session* session, SourceManager* sourceManager, RefPtr<IRModule>& moduleOut);

        /// Get the representation cache
    StringRepresentationCache& getStringRepresentationCache() { return m_stringRepresentationCache; }
    
    IRSerialReader():
        m_serialData(nullptr),
        m_module(nullptr)
    {
    }

    protected:

    static Result _skip(const IRSerialBinary::Chunk& chunk, Stream* stream, int64_t* remainingBytesInOut);

    StringRepresentationCache m_stringRepresentationCache;

    const IRSerialData* m_serialData;
    IRModule* m_module;
};

struct IRSerialUtil
{
        /// Produces an instruction list which is in same order as written through IRSerialWriter
    static void calcInstructionList(IRModule* module, List<IRInst*>& instsOut);

        /// Verify serialization
    static SlangResult verifySerialize(IRModule* module, Session* session, SourceManager* sourceManager, IRSerialBinary::CompressionType compressionType, IRSerialWriter::OptionFlags optionFlags);
};


} // namespace Slang

#endif
