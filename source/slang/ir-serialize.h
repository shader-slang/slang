// ir-serialize.h
#ifndef SLANG_IR_SERIALIZE_H_INCLUDED
#define SLANG_IR_SERIALIZE_H_INCLUDED

#include "../core/basic.h"
#include "../core/stream.h"

#include "ir.h"

// For TranslationUnitRequest
#include "compiler.h"

namespace Slang {

// Pre-declare
class Name;

struct IRSerialData
{
    enum class InstIndex : uint32_t;
    enum class StringIndex : uint32_t;
    enum class ArrayIndex : uint32_t;

    enum class RawSourceLoc : SourceLoc::RawValue;             ///< This is just to copy over source loc data (ie not strictly serialize)
    enum class StringOffset : uint32_t;             ///< Offset into the m_stringsBuffer
    
    typedef uint32_t SizeType;

    static const StringIndex kNullStringIndex = StringIndex(0);
    static const StringIndex kEmptyStringIndex = StringIndex(1);

    /// A run of instructions
    struct InstRun
    {
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

        /// Calculate the amount of memory used by this IRSerialData
    size_t calcSizeInBytes() const;

    /// Ctor
    IRSerialData() :
        m_decorationBaseIndex(0)
    {}

    List<Inst> m_insts;                         ///< The instructions

    List<InstRun> m_childRuns;                  ///< Holds the information about children that belong to an instruction
    List<InstRun> m_decorationRuns;             ///< Holds instruction decorations    

    List<InstIndex> m_externalOperands;         ///< Holds external operands (for instructions with more than kNumOperands)

    List<char> m_strings;                       ///< All strings. Indexed into by StringIndex

    List<RawSourceLoc> m_rawSourceLocs;         ///< A source location per instruction (saved without modification from IRInst)s

    static const PayloadInfo s_payloadInfos[int(Inst::PayloadType::CountOf)];
    
    int m_decorationBaseIndex;                  ///< All decorations insts are at indices >= to this value
};

// --------------------------------------------------------------------------
SLANG_FORCE_INLINE int IRSerialData::Inst::getNumOperands() const
{
    return (m_payloadType == PayloadType::OperandExternal) ? m_payload.m_externalOperand.m_size : s_payloadInfos[int(m_payloadType)].m_numOperands;
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

struct IRSerialBinary
{
    // http://fileformats.archiveteam.org/wiki/RIFF
    // http://www.fileformat.info/format/riff/egff.htm

    struct Chunk
    {
        uint32_t m_type;
        uint32_t m_size;
    };
    static const uint32_t kRiffFourCc = SLANG_FOUR_CC('R', 'I', 'F', 'F');
    static const uint32_t kSlangFourCc = SLANG_FOUR_CC('S', 'L', 'N', 'G');             ///< Holds all the slang specific chunks

    static const uint32_t kInstFourCc = SLANG_FOUR_CC('S', 'L', 'i', 'n');
    static const uint32_t kDecoratorRunFourCc = SLANG_FOUR_CC('S', 'L', 'd', 'r');
    static const uint32_t kChildRunFourCc = SLANG_FOUR_CC('S', 'L', 'c', 'r');
    static const uint32_t kExternalOperandsFourCc = SLANG_FOUR_CC('S', 'L', 'e', 'o');
    static const uint32_t kStringFourCc = SLANG_FOUR_CC('S', 'L', 's', 't');
        /// 4 bytes per entry
    static const uint32_t kUInt32SourceLocFourCc = SLANG_FOUR_CC('S', 'r', 's', '4');
        /// 8 bytes per entry
    static const uint32_t kUInt64SourceLocFourCc = SLANG_FOUR_CC('S', 'r', 's', '8');

    struct SlangHeader
    {
        Chunk m_chunk;
        uint32_t m_decorationBase;
    };
    struct ArrayHeader
    {
        Chunk m_chunk;
        uint32_t m_numEntries;
    };
};


struct IRSerialWriter
{
    typedef IRSerialData Ser;

    struct OptionFlag
    {
        typedef uint32_t Type;
        enum Enum: Type
        {
            RawSourceLocation = 1,
        };
    };
    typedef OptionFlag::Type OptionFlags;

    Result write(IRModule* module, SourceManager* sourceManager, OptionFlags options, IRSerialData* serialData);

    static Result writeStream(const IRSerialData& data, Stream* stream);

        /// Get a slice from an index
    UnownedStringSlice getStringSlice(Ser::StringIndex index) const;

    /// Get an instruction index from an instruction
    Ser::InstIndex getInstIndex(IRInst* inst) const { return inst ? Ser::InstIndex(m_instMap[inst]) : Ser::InstIndex(0); }

    Ser::StringIndex getStringIndex(StringRepresentation* string); 
    Ser::StringIndex getStringIndex(const UnownedStringSlice& string);
    Ser::StringIndex getStringIndex(Name* name);
    Ser::StringIndex getStringIndex(const char* chars);
    
    IRSerialWriter() :
        m_serialData(nullptr)
    {}

protected:
    void _addInstruction(IRInst* inst);

    List<IRInst*> m_insts;                              ///< Instructions in same order as stored in the 

    List<IRDecoration*> m_decorations;                  ///< Holds all decorations in order of the instructions as found
    List<IRInst*> m_instWithFirstDecoration;            ///< All decorations are held in this order after all the regular instructions

    Dictionary<IRInst*, Ser::InstIndex> m_instMap;      ///< Map an instruction to an instruction index

    List<Ser::StringOffset> m_stringStarts;                      ///< Offset for each string index into the m_strings 

    // TODO (JS):
    // We could perhaps improve this, if we stored at string indices (when linearized) the StringRepresentation
    // Doing so would mean if a String or Name was looked up we wouldn't have to re-allocate on the arena 
    Dictionary<UnownedStringSlice, Ser::StringIndex> m_stringMap;       ///< String map
    List<RefPtr<StringRepresentation> > m_scopeStrings;                 ///< 
    
    IRSerialData* m_serialData;                               ///< Where the data is stored
};

struct IRSerialReader
{
    typedef IRSerialData Ser;

        /// Read a stream to fill in dataOut IRSerialData
    static Result readStream(Stream* stream, IRSerialData* dataOut);

        /// Read a module from serial data
    Result read(const IRSerialData& data, Session* session, RefPtr<IRModule>& moduleOut);

    Name* getName(Ser::StringIndex index);
    String getString(Ser::StringIndex index);
    StringRepresentation* getStringRepresentation(Ser::StringIndex index);
    UnownedStringSlice getStringSlice(Ser::StringIndex index) { return getStringSlice(m_stringStarts[int(index)]); }
    char* getCStr(Ser::StringIndex index);

    UnownedStringSlice getStringSlice(Ser::StringOffset offset);

    IRSerialReader():
        m_serialData(nullptr),
        m_module(nullptr)
    {
    }

    protected:

    void _calcStringStarts();
    IRDecoration* _createDecoration(const Ser::Inst& srcIns);
    static Result _skip(const IRSerialBinary::Chunk& chunk, Stream* stream, int64_t* remainingBytesInOut);

    List<Ser::StringOffset> m_stringStarts;
    List<StringRepresentation*> m_stringRepresentationCache;

    const IRSerialData* m_serialData;
    IRModule* m_module;
};


Result serializeModule(IRModule* module, SourceManager* sourceManager, Stream* stream);
Result readModule(Session* session, Stream* stream, RefPtr<IRModule>& moduleOut);

} // namespace Slang

#endif
