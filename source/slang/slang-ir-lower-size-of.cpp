#include "slang-ir-lower-size-of.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "slang-ir-layout.h"

namespace Slang
{

struct SizeOfLikeLoweringContext
{    
    void _addToWorkList(IRInst* inst)
    {
        if (!findOuterGeneric(inst) && !m_workList.contains(inst))
        {
            m_workList.add(inst);
        }
    }

    void _processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_AlignOf:
        case kIROp_SizeOf:
            _processSizeOfLike(inst);
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        _addToWorkList(m_module->getModuleInst());

        while (m_workList.getCount() != 0)
        {
            IRInst* inst = m_workList.getLast();
            m_workList.removeLast();

            _processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                _addToWorkList(child);
            }
        }
    }

    void _processSizeOfLike(IRInst* sizeOfLikeInst)
    {
        auto typeOperand = as<IRType>(sizeOfLikeInst->getOperand(0));

        IRSizeAndAlignment sizeAndAlignment;

        if (SLANG_FAILED(getNaturalSizeAndAlignment(m_targetProgram->getOptionSet(), typeOperand, &sizeAndAlignment)))
        {
            // Output a diagnostic failure
            if(sizeOfLikeInst->getOp() == kIROp_AlignOf)
            {
                m_sink->diagnose(sizeOfLikeInst, Diagnostics::unableToAlignOf, typeOperand);
            }
            else
            {
                m_sink->diagnose(sizeOfLikeInst, Diagnostics::unableToSizeOf, typeOperand);
            }

            return;
        }

        IRBuilder builder(m_module);

        const auto value = (sizeOfLikeInst->getOp() == kIROp_AlignOf) ? 
            sizeAndAlignment.alignment : 
            sizeAndAlignment.size;

        auto valueInst = builder.getIntValue(sizeOfLikeInst->getDataType(), value); 

        // Replace all uses of sizeOfLikeInst with the value
        sizeOfLikeInst->replaceUsesWith(valueInst);
        // We don't need the instruction any more
        sizeOfLikeInst->removeAndDeallocate();
    }

    SizeOfLikeLoweringContext(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink):
        m_module(module),
        m_targetProgram(targetProgram),
        m_sink(sink)
    {
    }

    TargetProgram* m_targetProgram;
    DiagnosticSink* m_sink;
    IRModule* m_module;
    OrderedHashSet<IRInst*> m_workList;
};

void lowerSizeOfLike(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink)
{ 
    SizeOfLikeLoweringContext context(targetProgram, module, sink);
    context.processModule();
}

} // namespace Slang
