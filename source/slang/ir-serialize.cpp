// ir-serialize.cpp
#include "ir-serialize.h"

namespace Slang {

// http://fileformats.archiveteam.org/wiki/RIFF
// http://www.fileformat.info/format/riff/egff.htm

enum class IRSerialInstIndex: uint32_t;
enum class IRSerialStringIndex: uint32_t;
enum class IRArrayIndex: uint32_t;
enum class IRSerialSourceLoc: uint32_t;
typedef uint32_t IRSerialSize;

struct Chunk
{
    uint32_t m_type;
    uint32_t m_size;
};

struct IRSerializer
{
    struct SerialChildInfo
    {
        IRSerialInstIndex m_parentIndex;            ///< The parent instruction
        IRSerialInstIndex m_startInstIndex;         ///< The index to the first instruction
        IRSerialSize m_numChildren;                 ///< The number of children
    };

    // Instruction...
    // 1 1 1 1 1 1
    struct SerialInst
    {
        uint8_t m_op;                       ///< For now one of IROp 
        uint8_t m_flags;                    ///< Flags
        uint16_t m_pad1;					///< Lot's of possible uses

        // If I have the parent index, it is not necessary to store all the children.
        // On the other hand I've arranged such that all of the children are in order, 
        // so all that is really needed is the first instruction and the amount of children, which whilst awkward saves space
        // I could just hold the 'children' as an array (and only use a single index)
        // 
        // Perhaps I should just store the children information separately, store the parent, the start index, and num children)

        IRSerialInstIndex m_resultTypeIndex;	//< 0 if has no type. The result type of this instruction
        
        IRSerialSourceLoc m_sourceLoc;          //< 0 if no location

        struct IndirectOperand
        {
            IRArrayIndex m_operandArrayHandle;
            IRArrayIndex m_decorationArrayHandle;
        };
        union
        {
            IRFloatingPointValue m_floatValue;
            IRIntegerValue m_intValue;
            IndirectOperand m_indirectOperand; 		///< When operands are indirect use this structure
            IRSerialInstIndex m_operands[2];	            //< For items that 2 or less operands it can use this.   
        };

        
    };

    Result init(IRModule* module);

    IRSerializer():
        m_module(nullptr)
    {}

    protected:
    void _addInstruction(IRInst* inst);

    List<IRInst*> m_insts;    
    List<IRDecoration*> m_decorations;
    List<IRInst*> m_instWithDecorations;

    Dictionary<IRInst*, IRSerialInstIndex> m_instMap;
    Dictionary<IRDecoration*, IRSerialInstIndex> m_decorationMap;   ///< We will store decorations in 'instructions' too

    List<SerialInst> m_serialInsts;

    List<IRSerialInstIndex> m_instructionArray;         ///< Holds lists of instructions
    List<SerialChildInfo> m_childInfos;                 ///< Holds the information about children that belong to an instruction
    List<SerialChildInfo> m_decorationInfos;            ///< Holds instruction decorations    

    IRModule* m_module;
};


#define SLANG_FOUR_CC(c0, c1, c2, c3) ((uint32_t(c0) << 24) | (uint32_t(c0) << 16) | (uint32_t(c0) << 8) | (uint32_t(c0) << 0)) 

void IRSerializer::_addInstruction(IRInst* inst)
{
    // It cannot already be in the map
    SLANG_ASSERT(!m_instMap.ContainsKey(inst));

    // Add to the map
    m_instMap.Add(inst, IRSerialInstIndex(m_insts.Count()));
    m_insts.Add(inst);

    // Add all the decorations, to the list. 
    // We don't add to the decoration map, as we only want to do this once all the instructions have been hit
    {
        IRDecoration* decor = inst->firstDecoration;
  
        const int initialNumDecor = int(m_decorations.Count());
        while (decor)
        {
            m_decorations.Add(decor);
            decor = decor->next;
        }
        
        const IRSerialSize numDecor = IRSerialSize(int(m_decorations.Count()) - initialNumDecor);
        if (numDecor)
        {
            SerialChildInfo info;
            info.m_parentIndex = m_instMap[inst];
            // NOTE! This isn't quite correct, as we need to correct for when all the instuctions are added, this is done at the end
            info.m_startInstIndex = IRSerialInstIndex(initialNumDecor);
            info.m_numChildren = numDecor;

            m_decorationInfos.Add(info);
        }
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
    
    m_instWithDecorations.Clear();

    // We reserve 0 for null
    m_insts.Clear();
    m_insts.Add(nullptr);

    // Reset
    m_instMap.Clear();
    m_decorations.Clear();
    m_decorationMap.Clear();
    
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

        const IRSerialInstIndex startChildInstIndex = IRSerialInstIndex(m_insts.Count());
        
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
        if (IRSerialInstIndex(m_insts.Count()) != startChildInstIndex)
        {
            // Look up the parent index
            IRSerialInstIndex parentInstIndex = m_instMap[parentInst];

            SerialChildInfo childInfo;
            childInfo.m_parentIndex = parentInstIndex;
            childInfo.m_startInstIndex = startChildInstIndex;
            childInfo.m_numChildren = IRSerialSize(m_insts.Count() - int(startChildInstIndex));

            m_childInfos.Add(childInfo);
        }
    }

    // Now fix the declorations 
    {
        const int decorationBaseIndex = int(m_insts.Count());
        const int numDecorInfos = int(m_decorationInfos.Count());

        for (int i = 0; i < numDecorInfos; ++i)
        {
            SerialChildInfo& info = m_decorationInfos[i];
            info.m_startInstIndex = IRSerialInstIndex(decorationBaseIndex + int(info.m_startInstIndex));
        }
    }

    // Need to set up the actual instuctions

    // Now need to do the declorations

    return SLANG_OK;
}

Result serializeModule(IRModule* module, Stream* stream)
{
   IRSerializer serializer;

   SLANG_RETURN_ON_FAIL(serializer.init(module));

   if (stream)
   {
        // 
        Chunk riffChunk;
        riffChunk.m_type = SLANG_FOUR_CC('R', 'I', 'F', 'F');
        riffChunk.m_size = 0;                                       ///< We need to work out the overall size

        // Write initial marker
        stream->Write(&riffChunk, sizeof(riffChunk));
    }

    return SLANG_OK;
}

} // namespace Slang
