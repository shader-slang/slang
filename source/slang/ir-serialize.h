// ir-serialize.h
#ifndef SLANG_IR_SERIALIZE_H_INCLUDED
#define SLANG_IR_SERIALIZE_H_INCLUDED

#include "../core/basic.h"
#include "../core/stream.h"

#include "../core/slang-memory-arena.h"

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
    enum class SourceLoc : uint32_t;
    typedef uint32_t SizeType;

    static const StringIndex kNullStringIndex = StringIndex(0);
    static const StringIndex kEmptyStringIndex = StringIndex(1);

    enum
    {
        kNumOperands = 2,
    };

    /// A run of instructions
    struct InstRun
    {
        InstIndex m_parentIndex;            ///< The parent instruction
        InstIndex m_startInstIndex;         ///< The index to the first instruction
        SizeType m_numChildren;                 ///< The number of children
    };

    // Instruction...
    // We can store SourceLoc values separately. Just store per index information.
    // Parent information is stored in m_childRuns
    // Decoration information is stored in m_decorationRuns
    struct Inst
    {
        enum class PayloadType : uint8_t
        {
            Empty,                          ///< Has no payload (or operands)
            Operand_1,                      ///< 1 Operand
            Operand_2,                      ///< 2 Operands
            ExternalOperand,                ///< Operands are held externally
            String_1,                       ///< 1 String
            String_2,                       ///< 2 Strings
            UInt32,                         ///< Holds an unsigned 32 bit integral (might represent a type)
            Float64,
            Int64,
            CountOf,
        };

            /// Get the number of operands
        int getNumOperands() const 
        {
            switch (m_payloadType)
            {
                default: /* fallthru */
                case PayloadType::Empty: return 0;
                case PayloadType::Operand_1: return 1;
                case PayloadType::Operand_2: return 2;
                case PayloadType::ExternalOperand: return m_payload.m_externalOperand.m_size;
            }
        }

        uint8_t m_op;                       ///< For now one of IROp 
        PayloadType m_payloadType;	 		///< The type of payload 
        uint16_t m_pad0;                    ///< Not currently used             

        InstIndex m_resultTypeIndex;	    //< 0 if has no type. The result type of this instruction

        struct ExternalOperandPayload
        {
            ArrayIndex m_arrayIndex;                        ///< Index into the m_externalOperands table
            SizeType m_size;                                ///< The amount of entries in that table
        };

        union Payload
        {
            double m_float64;
            int64_t m_int64;
            uint32_t m_uint32;                              ///< Unsigned integral value
            IRFloatingPointValue m_float;              ///< Floating point value
            IRIntegerValue m_int;                      ///< Integral value
            InstIndex m_operands[kNumOperands];	            ///< For items that 2 or less operands it can use this.  
            StringIndex m_stringIndices[kNumOperands];
            ExternalOperandPayload m_externalOperand;              ///< Operands are stored in an an index of an operand array 
        };

        Payload m_payload;
    };

    /// Clear to initial state
    void clear()
    {
        // First Instruction is null
        m_insts.SetSize(1);
        memset(&m_insts[0], 0, sizeof(Inst));

        m_childRuns.Clear();
        m_decorationRuns.Clear();
        m_externalOperands.Clear();

        m_strings.SetSize(2);
        m_strings[int(kNullStringIndex)] = 0;
        m_strings[int(kEmptyStringIndex)] = 0;

        m_decorationBaseIndex = 0;
    }

    int getOperands(const Inst& inst, const InstIndex** operandsOut) const
    {
        switch (inst.m_payloadType)
        {
            default:
            case Inst::PayloadType::Empty:
            {
                *operandsOut = nullptr;
                return 0;
            }
            case Inst::PayloadType::Operand_1:
            case Inst::PayloadType::Operand_2:
            {
                *operandsOut = inst.m_payload.m_operands;
                return int(inst.m_payloadType) - int(Inst::PayloadType::Empty);
            }
            case Inst::PayloadType::ExternalOperand:
            {
                *operandsOut = m_externalOperands.begin() + int(inst.m_payload.m_externalOperand.m_arrayIndex);
                return int(inst.m_payload.m_externalOperand.m_size);
            }
        }
    }

    /// Get a slice from an index
    UnownedStringSlice getStringSlice(StringIndex index) const;

    /// Ctor
    IRSerialData() :
        m_decorationBaseIndex(0)
    {}


    List<Inst> m_insts;                         ///< The instructions

    List<InstRun> m_childRuns;                  ///< Holds the information about children that belong to an instruction
    List<InstRun> m_decorationRuns;             ///< Holds instruction decorations    

    List<InstIndex> m_externalOperands;         ///< Holds external operands (for instructions with more than kNumOperands)

    List<char> m_strings;                       ///< All strings. Indexed into by StringIndex

    int m_decorationBaseIndex;                  ///< All decorations insts are at indices >= to this value
};

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

    Result write(IRModule* module, IRSerialData* serialData);

    static Result writeStream(const IRSerialData& data, Stream* stream);

    /// Get an instruction index from an instruction
    Ser::InstIndex getInstIndex(IRInst* inst) const { return inst ? Ser::InstIndex(m_instMap[inst]) : Ser::InstIndex(0); }

    Ser::StringIndex getStringIndex(StringRepresentation* string) { return string ? getStringIndex(StringRepresentation::asSlice(string)) : Ser::kNullStringIndex; }
    Ser::StringIndex getStringIndex(const UnownedStringSlice& string);
    Ser::StringIndex getStringIndex(Name* name);
    Ser::StringIndex getStringIndex(const char* chars) { return chars ? getStringIndex(UnownedStringSlice(chars)) : Ser::kNullStringIndex; }

    IRSerialWriter() :
        m_serialData(nullptr),
        m_arena(1024 * 4)
    {}

protected:
    void _addInstruction(IRInst* inst);

    List<IRInst*> m_insts;                              ///< Instructions in same order as stored in the 

    List<IRDecoration*> m_decorations;                  ///< Holds all decorations in order of the instructions as found
    List<IRInst*> m_instWithFirstDecoration;            ///< All decorations are held in this order after all the regular instructions

    Dictionary<IRInst*, Ser::InstIndex> m_instMap;      ///< Map an instruction to an instruction index

    Dictionary<UnownedStringSlice, Ser::StringIndex> m_stringMap;       ///< String map

    MemoryArena m_arena;

    IRSerialData* m_serialData;                               ///< Where the data is stored
};

struct IRSerialReader
{
    typedef IRSerialData Ser;

        /// Read a stream to fill in dataOut IRSerialData
    static Result readStream(Stream* stream, IRSerialData* dataOut);

        /// Read a module from serial data
    Result read(const IRSerialData& data, TranslationUnitRequest* translationUnit, IRModule** moduleOut);

    Name* getName(Ser::StringIndex index);
    String getString(Ser::StringIndex index);
    UnownedStringSlice getStringSlice(Ser::StringIndex index);
    StringRepresentation* getStringRepresentation(Ser::StringIndex index);
    char* getCStr(Ser::StringIndex index);

        /// Given a string index, gives a linear position to it, or -1 if not found
    int findStringLinearIndex(Ser::StringIndex index);

    IRSerialReader():
        m_serialData(nullptr),
        m_module(nullptr)
    {
    }

    protected:

    void _calcStringStarts();

    List<Ser::StringIndex> m_stringStarts;
    List<StringRepresentation*> m_stringRepresentationCache;

    const IRSerialData* m_serialData;
    IRModule* m_module;
};


Result serializeModule(IRModule* module, Stream* stream);
Result readModule(TranslationUnitRequest* translationUnit, Stream* stream, IRModule** moduleOut);

} // namespace Slang

#endif
