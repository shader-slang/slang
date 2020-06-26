// slang-ir-serialize-types.h
#ifndef SLANG_IR_SERIALIZE_TYPES_H_INCLUDED
#define SLANG_IR_SERIALIZE_TYPES_H_INCLUDED

#include "../core/slang-riff.h"
#include "../core/slang-string-slice-pool.h"
#include "../core/slang-array-view.h"

#include "slang-name.h"
#include "slang-source-loc.h"

#include "slang-ir.h"

namespace Slang {

enum class IRSerialCompressionType : uint8_t
{
    None,
    VariableByteLite,
};

class StringRepresentationCache
{
    // TODO: The name of this type is no longer accurate to its function.
    // It doesn't *cache* anything, and instead just provides convenient
    // access to the contents of a serialized string table.

    public:
    typedef StringSlicePool::Handle Handle;

    struct Entry
    {
        uint32_t m_startIndex;
        uint32_t m_numChars;
    };

        /// Get as a string slice
    UnownedStringSlice getStringSlice(Handle handle) const;

        /// Initialize a cache to use a string table
    void init(const List<char>* stringTable);

        /// Ctor
    StringRepresentationCache(); 
    
    protected:
    const List<char>* m_stringTable;
    List<Entry> m_entries;
};

struct SerialStringTableUtil
{
        /// Convert a pool into a string table
    static void encodeStringTable(const StringSlicePool& pool, List<char>& stringTable);
    static void encodeStringTable(const ConstArrayView<UnownedStringSlice>& slices, List<char>& stringTable);
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

// Replace first char with 's'
#define SLANG_MAKE_COMPRESSED_FOUR_CC(fourCc) SLANG_FOUR_CC_REPLACE_FIRST_CHAR(fourCc, 's')

struct IRSerialBinary
{
    
    static const FourCC kRiffFourCc = RiffFourCC::kRiff;

    static const FourCC kSlangModuleListFourCc = SLANG_FOUR_CC('S', 'L', 'm', 'l');

    static const FourCC kSlangModuleFourCc = SLANG_FOUR_CC('S', 'L', 'm', 'd');             ///< Holds all the slang specific chunks

    static const FourCC kSlangModuleHeaderFourCc = SLANG_FOUR_CC('S', 'L', 'h', 'd');

    /* NOTE! All FourCC that can be compressed must start with capital 'S', because compressed version is the same FourCC
    with the 'S' replaced with 's' */

    static const FourCC kInstFourCc = SLANG_FOUR_CC('S', 'L', 'i', 'n');
    static const FourCC kChildRunFourCc = SLANG_FOUR_CC('S', 'L', 'c', 'r');
    static const FourCC kExternalOperandsFourCc = SLANG_FOUR_CC('S', 'L', 'e', 'o');

    static const FourCC kCompressedInstFourCc = SLANG_MAKE_COMPRESSED_FOUR_CC(kInstFourCc);
    static const FourCC kCompressedChildRunFourCc = SLANG_MAKE_COMPRESSED_FOUR_CC(kChildRunFourCc);
    static const FourCC kCompressedExternalOperandsFourCc = SLANG_MAKE_COMPRESSED_FOUR_CC(kExternalOperandsFourCc);


    static const FourCC kStringFourCc = SLANG_FOUR_CC('S', 'L', 's', 't');

    static const FourCC kUInt32SourceLocFourCc = SLANG_FOUR_CC('S', 'r', 's', '4');

    static const FourCC kDebugStringFourCc = SLANG_FOUR_CC('S', 'd', 's', 't');
    static const FourCC kDebugLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'l', 'n');
    static const FourCC kDebugAdjustedLineInfoFourCc = SLANG_FOUR_CC('S', 'd', 'a', 'l');
    static const FourCC kDebugSourceInfoFourCc = SLANG_FOUR_CC('S', 'd', 's', 'o');
    static const FourCC kDebugSourceLocRunFourCc = SLANG_FOUR_CC('S', 'd', 's', 'r');

    static const FourCC kEntryPointFourCc = SLANG_FOUR_CC('E', 'P', 'n', 't');

    typedef IRSerialCompressionType CompressionType;

    struct ModuleHeader
    {
        uint32_t compressionType;         ///< Holds the compression type used (if used at all)
    };
    struct ArrayHeader
    {
        uint32_t numEntries;
    };
    struct CompressedArrayHeader
    {
        uint32_t numEntries;              ///< The number of entries
        uint32_t numCompressedEntries;    ///< The amount of compressed entries
    };

};

struct IRSerialTypeUtil
{
        /// Given text, finds the compression type
    static SlangResult parseCompressionType(const UnownedStringSlice& text, IRSerialCompressionType& outType);
        /// Given a compression type, return text
    static UnownedStringSlice getText(IRSerialCompressionType type);
};


} // namespace Slang

#endif
