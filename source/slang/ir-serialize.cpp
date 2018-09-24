// ir-serialize.cpp
#include "ir-serialize.h"

#include "../core/text-io.h"
#include "../core/slang-memory-arena.h"

#include "ir-insts.h"

namespace Slang {

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

        /// Get a slice from an index
    UnownedStringSlice getStringSlice(StringIndex index) const;

        /// Ctor
    IRSerialData():
        m_decorationBaseIndex(0)
    {}


    List<Inst> m_insts;                         ///< The instructions

    List<InstRun> m_childRuns;                  ///< Holds the information about children that belong to an instruction
    List<InstRun> m_decorationRuns;             ///< Holds instruction decorations    

    List<InstIndex> m_externalOperands;         ///< Holds external operands (for instructions with more than kNumOperands)

    List<char> m_strings;                       ///< All strings. Indexed into by StringIndex

    int m_decorationBaseIndex;                  ///< All decorations insts are at indices >= to this value
};

#define SLANG_FOUR_CC(c0, c1, c2, c3) ((uint32_t(c0) << 24) | (uint32_t(c0) << 16) | (uint32_t(c0) << 8) | (uint32_t(c0) << 0)) 

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
    Ser::StringIndex getStringIndex(Name* name) { return name ? getStringIndex(name->text.getUnownedSlice()) : Ser::kNullStringIndex; }
    Ser::StringIndex getStringIndex(const char* chars) { return chars ? getStringIndex(UnownedStringSlice(chars)) : Ser::kNullStringIndex; }

    IRSerialWriter():
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



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

UnownedStringSlice IRSerialData::getStringSlice(StringIndex index) const
{
    if (index == StringIndex(0))
    {
        return UnownedStringSlice(m_strings.begin() + 1, UInt(0));
    }

    const char* prefix = m_strings.begin() + int(index);

    struct Reader
    {
        char operator()(int pos) const { SLANG_UNUSED(pos); return *m_pos++; } 
        Reader(const char* pos):m_pos(pos) {}
        mutable const char* m_pos;
    };

    Reader reader(prefix);
    const int len = GetUnicodePointFromUTF8(reader);
    return UnownedStringSlice(reader.m_pos, len);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void IRSerialWriter::_addInstruction(IRInst* inst)
{
    // It cannot already be in the map
    SLANG_ASSERT(!m_instMap.ContainsKey(inst));

    // Add to the map
    m_instMap.Add(inst, Ser::InstIndex(m_insts.Count()));
    m_insts.Add(inst);

    // Add all the decorations, to the list. 
    // We don't add to the decoration map, as we only want to do this once all the instructions have been hit
    if (inst->firstDecoration)
    {
        m_instWithFirstDecoration.Add(inst);

        IRDecoration* decor = inst->firstDecoration;
  
        const int initialNumDecor = int(m_decorations.Count());
        while (decor)
        {
            m_decorations.Add(decor);
            decor = decor->next;
        }
        
        const Ser::SizeType numDecor = Ser::SizeType(int(m_decorations.Count()) - initialNumDecor);
        
        Ser::InstRun run;
        run.m_parentIndex = m_instMap[inst];

        // NOTE! This isn't quite correct, as we need to correct for when all the instructions are added, this is done at the end
        run.m_startInstIndex = Ser::InstIndex(initialNumDecor);
        run.m_numChildren = numDecor;

        m_serialData->m_decorationRuns.Add(run);
    }
}

Result IRSerialWriter::write(IRModule* module, IRSerialData* serialInfo)
{
    // Lets find all of the instructions
    // Each instruction can only have one parent
    // That being the case if we traverse the module in breadth first order (following children specifically)
    // Then all 'child' nodes will be in order, and thus we only have to store the start instruction
    // and the amount of instructions 

    m_serialData = serialInfo;

    serialInfo->clear();

    // We reserve 0 for null
    m_insts.Clear();
    m_insts.Add(nullptr);

    // Reset
    m_instMap.Clear();
    m_decorations.Clear();
    
    // Stack for parentInst
    List<IRParentInst*> parentInstStack;
  
    IRModuleInst* moduleInst = module->getModuleInst();
    parentInstStack.Add(moduleInst);

    // Add to the map
    _addInstruction(moduleInst);

    // Traverse all of the instructions
    while (parentInstStack.Count())
    {
        // If it's in the stack it is assumed it is already in the inst map
        IRParentInst* parentInst = parentInstStack.Last();
        parentInstStack.RemoveLast();
        SLANG_ASSERT(m_instMap.ContainsKey(parentInst));

        
        // Okay we go through each of the children in order. If they are IRInstParent derived, we add to stack to process later 
        // cos we want breadth first...

        const Ser::InstIndex startChildInstIndex = Ser::InstIndex(m_insts.Count());
        
        IRInstListBase childrenList = parentInst->getChildren();
        for (IRInst* child : childrenList)
        {
            // This instruction can't be in the map...
            SLANG_ASSERT(!m_instMap.ContainsKey(child));

            _addInstruction(child);
            
            IRParentInst* childAsParent = as<IRParentInst>(child);
            if (childAsParent)
            {
                parentInstStack.Add(childAsParent);
            }
        }

        // If it had any children, then store the information about it
        if (Ser::InstIndex(m_insts.Count()) != startChildInstIndex)
        {
            Ser::InstRun run;
            run.m_parentIndex = m_instMap[parentInst];
            run.m_startInstIndex = startChildInstIndex;
            run.m_numChildren = Ser::SizeType(m_insts.Count() - int(startChildInstIndex));

            m_serialData->m_childRuns.Add(run);
        }
    }

    // Now fix the decorations 
    {
        const int decorationBaseIndex = int(m_insts.Count());
        m_serialData->m_decorationBaseIndex = decorationBaseIndex;
        const int numDecorRuns = int(m_serialData->m_decorationRuns.Count());

        // Work out the total num of decoration
        int totalNumDecorations = 0;
        if (numDecorRuns)
        {
            const auto& lastDecorInfo = m_serialData->m_decorationRuns.Last();
            totalNumDecorations = int(lastDecorInfo.m_startInstIndex) + lastDecorInfo.m_numChildren;
        }

        // Fix the indices
        for (int i = 0; i < numDecorRuns; ++i)
        {
            Ser::InstRun& info = m_serialData->m_decorationRuns[i];
            info.m_startInstIndex = Ser::InstIndex(decorationBaseIndex + int(info.m_startInstIndex));
        }

        // Set to the right size
        m_serialData->m_insts.SetSize(decorationBaseIndex + totalNumDecorations);
        // Clear all instructions
        memset(m_serialData->m_insts.begin(), 0, sizeof(Ser::Inst) * m_serialData->m_insts.Count());
    }

    // Need to set up the actual instructions
    {
        const int numInsts = int(m_insts.Count());

        for (int i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            Ser::Inst& dstInst = m_serialData->m_insts[i];
            
            dstInst.m_op = uint8_t(srcInst->op);
            dstInst.m_payloadType = Ser::Inst::PayloadType::Empty;
            
            dstInst.m_resultTypeIndex = getInstIndex(srcInst->getFullType());

            // Need to special case 
            if (srcInst->op != (srcInst->op & kIROpMeta_OpMask))
            {
                // For now ignore
                continue;
            }
            
            IRConstant* irConst = as<IRConstant>(srcInst);    
            if (irConst)
            {
                switch (srcInst->op)
                {
                    case kIROp_StringLit:
                    {
                        auto stringLit = static_cast<IRStringLit*>(srcInst);
                        dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                        dstInst.m_payload.m_stringIndices[0] = getStringIndex(stringLit->getStringSlice());
                        break;
                    }
                    case kIROp_IntLit:
                    {
                        dstInst.m_payloadType = Ser::Inst::PayloadType::Int64;
                        dstInst.m_payload.m_int64 = irConst->value.intVal;
                        break;
                    }
                    case kIROp_FloatLit:
                    {
                        dstInst.m_payloadType = Ser::Inst::PayloadType::Float64;
                        dstInst.m_payload.m_float64 = irConst->value.floatVal; 
                        break;
                    }
                    case kIROp_boolConst:
                    {
                        dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                        dstInst.m_payload.m_uint32 = irConst->value.intVal ? 1 : 0;
                        break;
                    }
                }
                
                continue;
            }

            const int numOperands = int(srcInst->operandCount);
            Ser::InstIndex* dstOperands = nullptr;

            if (numOperands <= Ser::kNumOperands)
            {
                dstOperands = dstInst.m_payload.m_operands;
                dstInst.m_payloadType = Ser::Inst::PayloadType(numOperands);
            }
            else
            {
                dstInst.m_payloadType = Ser::Inst::PayloadType::ExternalOperand;

                int operandArrayBaseIndex = int(m_serialData->m_externalOperands.Count());
                m_serialData->m_externalOperands.SetSize(operandArrayBaseIndex + numOperands);

                dstOperands = m_serialData->m_externalOperands.begin() + operandArrayBaseIndex; 

                auto& externalOperands = dstInst.m_payload.m_externalOperand;
                externalOperands.m_arrayIndex = Ser::ArrayIndex(operandArrayBaseIndex);
                externalOperands.m_size = Ser::SizeType(numOperands);
            }

            for (int j = 0; j < numOperands; ++j)
            {
                const Ser::InstIndex dstInstIndex = getInstIndex(srcInst->getOperand(j));
                dstOperands[j] = dstInstIndex;
            }
        }
    }

    // Now need to do the decorations

    {
        const int decorationBaseIndex = m_serialData->m_decorationBaseIndex;
        const int numDecor = int(m_decorations.Count());
        SLANG_ASSERT(decorationBaseIndex + numDecor == m_serialData->m_insts.Count());

        // Have to be able to store in a byte!
        SLANG_COMPILE_TIME_ASSERT(kIROpCount + kIRDecorationOp_CountOf < 0x100);

        for (int i = 0; i < numDecor; ++i)
        {
            IRDecoration* srcDecor = m_decorations[i];
            Ser::Inst& dstInst = m_serialData->m_insts[decorationBaseIndex + i];

            dstInst.m_op = uint8_t(kIROpCount + srcDecor->op);

            switch (srcDecor->op)
            {
                case kIRDecorationOp_HighLevelDecl:
                {
                    // TODO!
                    // Decl* decl;
                    break;
                }
                case kIRDecorationOp_Layout:
                {
                    // TODO!
                    // Layout* layout;
                    break;
                }
                case kIRDecorationOp_LoopControl:
                {
                    auto loopDecor = static_cast<IRLoopControlDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::UInt32;
                    dstInst.m_payload.m_uint32 = uint32_t(loopDecor->mode);
                    break;
                }
                case kIRDecorationOp_Target:
                {
                    auto targetDecor = static_cast<IRTargetDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    break;
                }
                case kIRDecorationOp_TargetIntrinsic:
                {
                    auto targetDecor = static_cast<IRTargetIntrinsicDecoration*>(srcDecor);
                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_2;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    dstInst.m_payload.m_stringIndices[1] = getStringIndex(targetDecor->definition);
                    break;
                }
                case kIRDecorationOp_GLSLOuterArray:
                {
                    auto arrayDecor = static_cast<IRGLSLOuterArrayDecoration*>(srcDecor);
                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(arrayDecor->outerArrayName);
                    break;
                }
                case kIRDecorationOp_Semantic:
                {
                    auto semanticDecor = static_cast<IRSemanticDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(semanticDecor->semanticName);
                    break;
                }
                case kIRDecorationOp_NameHint:
                {
                    auto nameDecor = static_cast<IRNameHintDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(nameDecor->name);
                    break;
                }
                default:
                {
                    SLANG_ASSERT(!"Unhandled decoration type");
                    return SLANG_FAIL;
                }
            }
        }
    }

    m_serialData = nullptr;

    return SLANG_OK;
}

IRSerialData::StringIndex IRSerialWriter::getStringIndex(const UnownedStringSlice& slice)
{
    const int len = int(slice.size());
    if (len <= 0)
    {
        return Ser::kEmptyStringIndex;
    }

    Ser::StringIndex index;
    if (m_stringMap.TryGetValue(slice, index))
    {
        return index;
    }

    // We need to write into the the string array
    char prefixBytes[6];
    const int numPrefixBytes = EncodeUnicodePointToUTF8(prefixBytes, len);
    const int baseIndex = int(m_serialData->m_strings.Count());

    m_serialData->m_strings.SetSize(baseIndex + numPrefixBytes + len);

    char* dst = m_serialData->m_strings.begin() + baseIndex;

    memcpy(dst, prefixBytes, numPrefixBytes);
    memcpy(dst + numPrefixBytes, slice.begin(), len);

    // Add to the map. Unfortunately we can't use the string storage in the array, because the address changes with size
    // so we have to store backing string in an arena
    const char* arenaChars = m_arena.allocateString(slice.begin(), len);
    m_stringMap.Add(UnownedStringSlice(arenaChars, len), Ser::StringIndex(baseIndex));

    return Ser::StringIndex(baseIndex);
}

template <typename T>
static size_t _calcChunkSize(const List<T>& array)
{
    typedef IRSerialBinary Bin;

    if (array.Count())
    {
        return sizeof(Bin::ArrayHeader) - sizeof(Bin::Chunk) + sizeof(T) * array.Count();
    }
    else
    {
        return 0;
    }
}

template <typename T>
Result _writeArrayChunk(uint32_t chunkId, const List<T>& array, Stream* stream)
{
    typedef IRSerialBinary Bin;

    if (array.Count() == 0)
    {
        return SLANG_OK;
    }

    size_t payloadSize = sizeof(Bin::ArrayHeader) - sizeof(Bin::Chunk) + sizeof(T) * array.Count();

    Bin::ArrayHeader header;
    header.m_chunk.m_type = chunkId;
    header.m_chunk.m_size = uint32_t(payloadSize);
    header.m_numEntries = uint32_t(array.Count());

    stream->Write(&header, sizeof(header));

    stream->Write(array.begin(), sizeof(T) * array.Count());

    // All chunks have sizes rounded to dword size
    if (payloadSize & 3)
    {
        const uint8_t pad[4] = { 0, 0, 0, 0 };
        // Pad outs
        int padSize = 4 - (payloadSize & 3);
        stream->Write(pad, padSize);
    }

    return SLANG_OK;
}

/* static */Result IRSerialWriter::writeStream(const IRSerialData& data, Stream* stream)
{
    size_t totalSize = 0;

    typedef IRSerialBinary Bin;

    totalSize += sizeof(Bin::SlangHeader) + 
        _calcChunkSize(data.m_insts) + 
        _calcChunkSize(data.m_childRuns) +
        _calcChunkSize(data.m_decorationRuns) +
        _calcChunkSize(data.m_externalOperands) +
        _calcChunkSize(data.m_strings);

    {
        Bin::Chunk riffHeader;
        riffHeader.m_type = Bin::kRiffFourCc;
        riffHeader.m_size = uint32_t(totalSize);

        stream->Write(&riffHeader, sizeof(riffHeader));
    }
    {
        Bin::SlangHeader slangHeader;
        slangHeader.m_chunk.m_type = Bin::kSlangFourCc;
        slangHeader.m_chunk.m_size = uint32_t(sizeof(slangHeader) - sizeof(Bin::Chunk));
        slangHeader.m_decorationBase = uint32_t(data.m_decorationBaseIndex);

        stream->Write(&slangHeader, sizeof(slangHeader));
    }

    _writeArrayChunk(Bin::kInstFourCc, data.m_insts, stream);
    _writeArrayChunk(Bin::kChildRunFourCc, data.m_childRuns, stream);
    _writeArrayChunk(Bin::kDecoratorRunFourCc, data.m_decorationRuns, stream);
    _writeArrayChunk(Bin::kExternalOperandsFourCc, data.m_externalOperands, stream);
    _writeArrayChunk(Bin::kStringFourCc, data.m_strings, stream);
    
    return SLANG_OK;
}

Result serializeModule(IRModule* module, Stream* stream)
{
   IRSerialWriter serializer;
   IRSerialData serialData;

   SLANG_RETURN_ON_FAIL(serializer.write(module, &serialData));

   if (stream)
   {
        SLANG_RETURN_ON_FAIL(IRSerialWriter::writeStream(serialData, stream));
    }

    return SLANG_OK;
}

} // namespace Slang
