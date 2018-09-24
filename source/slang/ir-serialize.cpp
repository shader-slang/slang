// ir-serialize.cpp
#include "ir-serialize.h"

#include "ir-insts.h"

namespace Slang {


struct IRSerializer
{
    // http://fileformats.archiveteam.org/wiki/RIFF
    // http://www.fileformat.info/format/riff/egff.htm

    enum class InstIndex : uint32_t;
    enum class StringIndex : uint32_t;
    enum class ArrayIndex : uint32_t;
    enum class SourceLoc : uint32_t;
    typedef uint32_t SizeType;

    struct Chunk
    {
        uint32_t m_type;
        uint32_t m_size;
    };

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
    // 
    struct SerialInst
    {
        enum class PayloadType : uint8_t
        {
            Empty,                          ///< Has no payload (or operands)
            Operand_1,                      ///< 1 Operand
            Operand_2,                      ///< 2 Operands
            ExternalOperand,                ///< Operands are held externally
            String_1,                       ///< 1 String
            String_2,                       ///< 2 Strings
            UnsignedIntegral,               ///< Holds an unsigned 32 bit integral (might represent a type)
        };

        uint8_t m_op;                       ///< For now one of IROp 
        PayloadType m_payloadType;	 		///< The type of payload 
        uint16_t m_pad0;                    ///< Not currently used             
        
        // If I have the parent index, it is not necessary to store all the children.
        // On the other hand I've arranged such that all of the children are in order, 
        // so all that is really needed is the first instruction and the amount of children, which whilst awkward saves space
        // I could just hold the 'children' as an array (and only use a single index)
        // 
        // Perhaps I should just store the children information separately, store the parent, the start index, and num children)
        InstIndex m_resultTypeIndex;	//< 0 if has no type. The result type of this instruction

        struct ExternalOperandPayload
        {
            ArrayIndex m_arrayIndex;                        ///< Index into the m_externalOperands table
            SizeType m_size;                                ///< The amount of entries in that table
        };
        struct UIntPayload
        {
            uint32_t m_value;
        };

        union Payload
        {
            UIntPayload m_uint;                             ///< Unsigned integral value
            IRFloatingPointValue m_floatValue;              ///< Floating point value
            IRIntegerValue m_intValue;                      ///< Integral value
            InstIndex m_operands[kNumOperands];	            ///< For items that 2 or less operands it can use this.  
            StringIndex m_stringIndices[kNumOperands];
            ExternalOperandPayload m_externalOperand;              ///< Operands are stored in an an index of an operand array 
        };

        Payload m_payload;
    };

    Result init(IRModule* module);

        /// Get an instruction index from an instruction
    InstIndex getInstIndex(IRInst* inst) const { return inst ? InstIndex(m_instMap[inst]) : InstIndex(0); }

    StringIndex getStringIndex(StringRepresentation* string) { SLANG_UNUSED(string); return StringIndex(0); }
    StringIndex getStringIndex(const UnownedStringSlice& string) { SLANG_UNUSED(string); return StringIndex(0); }
    StringIndex getStringIndex(Name* name) { return name ? getStringIndex(name->text.getUnownedSlice()) : StringIndex(0); }

    IRSerializer():
        m_module(nullptr)
    {}

    protected:
    void _addInstruction(IRInst* inst);

    List<IRInst*> m_insts;    

    List<IRDecoration*> m_decorations;                  ///< Holds all decorations in order of the instructions as found
    List<IRInst*> m_instWithFirstDecoration;            ///< All decorations are held in this order after all the regular instructions
    
    Dictionary<IRInst*, InstIndex> m_instMap;   ///< Map an instruction to an instruction index
    
    List<SerialInst> m_serialInsts;

    List<InstRun> m_childInfos;                 ///< Holds the information about children that belong to an instruction
    List<InstRun> m_decorationInfos;            ///< Holds instruction decorations    

    List<InstIndex> m_externalOperands;         ///< Holds external operands (for instructions with more than kNumOperands)

    // We can store SourceLoc values separately. Just store per index information.

    int m_decorationBaseIndex;

    IRModule* m_module;
};


#define SLANG_FOUR_CC(c0, c1, c2, c3) ((uint32_t(c0) << 24) | (uint32_t(c0) << 16) | (uint32_t(c0) << 8) | (uint32_t(c0) << 0)) 

void IRSerializer::_addInstruction(IRInst* inst)
{
    // It cannot already be in the map
    SLANG_ASSERT(!m_instMap.ContainsKey(inst));

    // Add to the map
    m_instMap.Add(inst, InstIndex(m_insts.Count()));
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
        
        const SizeType numDecor = SizeType(int(m_decorations.Count()) - initialNumDecor);
        
        InstRun info;
        info.m_parentIndex = m_instMap[inst];

        // NOTE! This isn't quite correct, as we need to correct for when all the instructions are added, this is done at the end
        info.m_startInstIndex = InstIndex(initialNumDecor);
        info.m_numChildren = numDecor;

        m_decorationInfos.Add(info);
    }
}

Result IRSerializer::init(IRModule* module)
{
    // Lets find all of the instructions
    // Each instruction can only have one parent
    // That being the case if we traverse the module in breadth first order (following children specifically)
    // Then all 'child' nodes will be in order, and thus we only have to store the start instruction
    // and the amount of instructions 

    m_serialInsts.Clear();
    
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

        const InstIndex startChildInstIndex = InstIndex(m_insts.Count());
        
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

        // If it had any children, then store the information it
        if (InstIndex(m_insts.Count()) != startChildInstIndex)
        {
            // Look up the parent index
            InstIndex parentInstIndex = m_instMap[parentInst];

            InstRun childInfo;
            childInfo.m_parentIndex = parentInstIndex;
            childInfo.m_startInstIndex = startChildInstIndex;
            childInfo.m_numChildren = SizeType(m_insts.Count() - int(startChildInstIndex));

            m_childInfos.Add(childInfo);
        }
    }

    // Now fix the decorations 
    {
        m_decorationBaseIndex = int(m_insts.Count());
        const int numDecorInfos = int(m_decorationInfos.Count());

        // Work out the total num of decoration
        int totalNumDecorations = 0;
        if (numDecorInfos)
        {
            const auto& lastDecorInfo = m_decorationInfos.Last();
            totalNumDecorations = int(lastDecorInfo.m_startInstIndex) + lastDecorInfo.m_numChildren;
        }

        // Fix the indices
        for (int i = 0; i < numDecorInfos; ++i)
        {
            InstRun& info = m_decorationInfos[i];
            info.m_startInstIndex = InstIndex(m_decorationBaseIndex + int(info.m_startInstIndex));
        }

        // Set to the right size
        m_serialInsts.SetSize(m_decorationBaseIndex + totalNumDecorations);
        // Clear all instructions
        memset(m_serialInsts.begin(), 0, sizeof(SerialInst) * m_serialInsts.Count());
    }

    // Need to set up the actual instructions
    {
        const int numInsts = int(m_insts.Count());

        for (int i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            SerialInst& dstInst = m_serialInsts[i];
            
            dstInst.m_op = uint8_t(srcInst->op);
            dstInst.m_payloadType = SerialInst::PayloadType::Empty;
            
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
                        dstInst.m_payloadType = SerialInst::PayloadType::String_1;
                        dstInst.m_payload.m_stringIndices[0] = getStringIndex(stringLit->getStringSlice());
                        break;
                    }
                    case kIROp_IntLit:
                    {
                    }
                    case kIROp_FloatLit:
                    {
                    }
                    case kIROp_boolConst:
                    {
                    }
                }
                
                continue;
            }

            const int numOperands = int(srcInst->operandCount);
            InstIndex* dstOperands = nullptr;

            if (numOperands <= kNumOperands)
            {
                dstOperands = dstInst.m_payload.m_operands;
                dstInst.m_payloadType = SerialInst::PayloadType(numOperands);
            }
            else
            {
                dstInst.m_payloadType = SerialInst::PayloadType::ExternalOperand;

                int operandArrayBaseIndex = int(m_externalOperands.Count());
                m_externalOperands.SetSize(operandArrayBaseIndex + numOperands);

                dstOperands = m_externalOperands.begin() + operandArrayBaseIndex; 

                auto& externalOperands = dstInst.m_payload.m_externalOperand;
                externalOperands.m_arrayIndex = ArrayIndex(operandArrayBaseIndex);
                externalOperands.m_size = SizeType(numOperands);
            }

            for (int j = 0; j < numOperands; ++j)
            {
                const InstIndex dstInstIndex = getInstIndex(srcInst->getOperand(j));
                dstOperands[j] = dstInstIndex;
            }
        }
    }

    // Now need to do the decorations

    {
        const int decorationBaseIndex = m_decorationBaseIndex;
        const int numDecor = int(m_decorations.Count());
        SLANG_ASSERT(decorationBaseIndex + numDecor == m_serialInsts.Count());

        // Have to be able to store in a byte!
        SLANG_COMPILE_TIME_ASSERT(kIROpCount + kIRDecorationOp_CountOf < 0x100);

        for (int i = 0; i < numDecor; ++i)
        {
            IRDecoration* srcDecor = m_decorations[i];
            SerialInst& dstInst = m_serialInsts[decorationBaseIndex + i];

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

                    dstInst.m_payloadType = SerialInst::PayloadType::UnsignedIntegral;
                    dstInst.m_payload.m_uint.m_value = uint32_t(loopDecor->mode);
                    break;
                }
                case kIRDecorationOp_Target:
                {
                    auto targetDecor = static_cast<IRTargetDecoration*>(srcDecor);

                    dstInst.m_payloadType = SerialInst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    break;
                }
                case kIRDecorationOp_TargetIntrinsic:
                {
                    auto targetDecor = static_cast<IRTargetIntrinsicDecoration*>(srcDecor);
                    dstInst.m_payloadType = SerialInst::PayloadType::String_2;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    dstInst.m_payload.m_stringIndices[1] = getStringIndex(targetDecor->definition);
                    break;
                }
                case kIRDecorationOp_GLSLOuterArray:
                {
                    auto arrayDecor = static_cast<IRGLSLOuterArrayDecoration*>(srcDecor);
                    dstInst.m_payloadType = SerialInst::PayloadType::String_1;

                    UnownedStringSlice slice = arrayDecor->outerArrayName ? UnownedStringSlice(arrayDecor->outerArrayName) : UnownedStringSlice();

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(slice);
                    break;
                }
                case kIRDecorationOp_Semantic:
                {
                    auto semanticDecor = static_cast<IRSemanticDecoration*>(srcDecor);

                    dstInst.m_payloadType = SerialInst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(semanticDecor->semanticName);
                    break;
                }
                case kIRDecorationOp_NameHint:
                {
                    auto nameDecor = static_cast<IRNameHintDecoration*>(srcDecor);

                    dstInst.m_payloadType = SerialInst::PayloadType::String_1;
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

    return SLANG_OK;
}

Result serializeModule(IRModule* module, Stream* stream)
{
   IRSerializer serializer;

   SLANG_RETURN_ON_FAIL(serializer.init(module));

   if (stream)
   {
        // 
        IRSerializer::Chunk riffChunk;
        riffChunk.m_type = SLANG_FOUR_CC('R', 'I', 'F', 'F');
        riffChunk.m_size = 0;                                       ///< We need to work out the overall size

        // Write initial marker
        stream->Write(&riffChunk, sizeof(riffChunk));
    }

    return SLANG_OK;
}

} // namespace Slang
