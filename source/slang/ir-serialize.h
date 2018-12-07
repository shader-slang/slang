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
    /// Converts a pool into a string table, appending the strings to the slices
    static void decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut);
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

    struct PayloadInfo
    {
        uint8_t m_numOperands;
        uint8_t m_numStrings;
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
 
    struct DebugSourceFile
    {
        uint32_t m_startLoc;                    ///< Start of the location range 
        uint32_t m_endLoc;                      ///< The end of the location range

        uint32_t m_pathIndex;                   ///< Path associated

        uint32_t m_numLocRuns;                  ///< The number of location runs associated with this source file
        uint32_t m_numLineOffsets;              ///< The number of offsets associated with the file
        uint32_t m_numDebugViewEntries;         ///< The number of debug view entries
    };

    struct DebugViewEntry
    {
        uint32_t m_startLoc;                    ///< Where does this entry begin?
        uint32_t m_pathIndex;                   ///< What is the presumed path for this entry. If 0 it means there is no path.
        int32_t m_lineAdjust;                   ///< The line adjustment
    };

    struct DebugLocRun
    {
        uint32_t m_sourceLoc;                   ///< The location
        uint32_t startInstIndex;                ///< The start instruction index
        uint32_t numInst;                       ///< The amount of instructions
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
    IRSerialData()
    {}

    List<Inst> m_insts;                         ///< The instructions

    List<InstRun> m_childRuns;                  ///< Holds the information about children that belong to an instruction

    List<InstIndex> m_externalOperands;         ///< Holds external operands (for instructions with more than kNumOperands)

    List<char> m_stringTable;                       ///< All strings. Indexed into by StringIndex

    List<RawSourceLoc> m_rawSourceLocs;         ///< A source location per instruction (saved without modification from IRInst)s

    List<DebugSourceFile> m_debugSourceFiles;   ///< The files associated 
    List<uint32_t> m_debugLineOffsets;          ///< All of the debug line offsets
    List<uint32_t> m_debugViewEntries;          ///< The debug view entries - that modify line meanings
    List<DebugLocRun> m_debugLocRuns;           ///< Maps source locations to instructions
    List<char> m_debugStrings;                  ///< All of the debug strings

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
        /// 4 bytes per entry
    static const uint32_t kUInt32SourceLocFourCc = SLANG_FOUR_CC('S', 'r', 's', '4');

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

    IRSerialWriter() :
        m_serialData(nullptr)
    {}

protected:
    void _addInstruction(IRInst* inst);
    
    List<IRInst*> m_insts;                              ///< Instructions in same order as stored in the 

    List<IRDecoration*> m_decorations;                  ///< Holds all decorations in order of the instructions as found
    List<IRInst*> m_instWithFirstDecoration;            ///< All decorations are held in this order after all the regular instructions

    Dictionary<IRInst*, Ser::InstIndex> m_instMap;      ///< Map an instruction to an instruction index

    StringSlicePool m_stringSlicePool;    
    IRSerialData* m_serialData;                         ///< Where the data is stored

    StringSlicePool m_debugStringSlicePool;             ///< Slices held just for debug usage
};

struct IRSerialReader
{
    typedef IRSerialData Ser;
    typedef StringRepresentationCache::Handle StringHandle;

        /// Read a stream to fill in dataOut IRSerialData
    static Result readStream(Stream* stream, IRSerialData* dataOut);

        /// Read a module from serial data
    Result read(const IRSerialData& data, Session* session, RefPtr<IRModule>& moduleOut);

        /// Get the representation cache
    StringRepresentationCache& getStringRepresentationCache() { return m_stringRepresentationCache; }
    
    IRSerialReader():
        m_serialData(nullptr),
        m_module(nullptr)
    {
    }

    protected:

    IRDecoration* _createDecoration(const Ser::Inst& srcIns);
    static Result _skip(const IRSerialBinary::Chunk& chunk, Stream* stream, int64_t* remainingBytesInOut);

    StringRepresentationCache m_stringRepresentationCache;

    const IRSerialData* m_serialData;
    IRModule* m_module;
};

} // namespace Slang

#endif
