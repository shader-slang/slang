#include "slang-ir-call-graph.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{

CallGraph::CallGraph(IRModule* module)
{
    build(module);
}

template<typename T, typename U>
static void addToReferenceMap(Dictionary<T, HashSet<U>>& map, T key, U value)
{
    if (auto set = map.tryGetValue(key))
    {
        set->add(value);
    }
    else
    {
        HashSet<U> newSet;
        newSet.add(value);
        map.add(key, _Move(newSet));
    }
}


void CallGraph::registerInstructionReference(IRInst* inst, IRFunc* entryPoint, IRFunc* parentFunc)
{
    addToReferenceMap(m_referencingEntryPoints, inst, entryPoint);
    addToReferenceMap(m_referencingFunctions, inst, parentFunc);
}

void CallGraph::registerCallReference(IRFunc* func, IRCall* call)
{
    addToReferenceMap(m_referencingCalls, func, call);
}

void CallGraph::build(IRModule* module)
{
    struct WorkItem
    {
        IRInst* inst;
        IRFunc* entryPoint;
        IRFunc* parentFunc;

        HashCode getHashCode() const
        {
            return combineHash(Slang::getHashCode(entryPoint), Slang::getHashCode(inst));
        }
        bool operator==(const WorkItem& other) const
        {
            return entryPoint == other.entryPoint && inst == other.inst;
        }
    };
    HashSet<WorkItem> workListSet;
    List<WorkItem> workList;
    auto addToWorkList = [&](WorkItem item)
    {
        if (workListSet.add(item))
            workList.add(item);
    };

    auto visit = [&](IRInst* inst, IRFunc* entryPoint, IRFunc* parentFunc)
    {
        if (auto code = as<IRGlobalValueWithCode>(inst))
        {
            registerInstructionReference(inst, entryPoint, parentFunc);

            if (auto func = as<IRFunc>(code))
            {
                parentFunc = func;
            }

            for (auto child : code->getChildren())
            {
                addToWorkList({child, entryPoint, parentFunc});
            }
            return;
        }

        switch (inst->getOp())
        {
        // Only these instruction types and `IRGlobalValueWithCode` instructions are registered to
        // the reference graph.
        case kIROp_Call:
            {
                auto call = as<IRCall>(inst);
                registerCallReference(as<IRFunc>(call->getCallee()), call);
                addToWorkList({call->getCallee(), entryPoint, parentFunc});
            }
            [[fallthrough]];
        case kIROp_GlobalParam:
        case kIROp_SPIRVAsmOperandBuiltinVar:
        case kIROp_ImplicitSystemValue:
            registerInstructionReference(inst, entryPoint, parentFunc);
            break;

        case kIROp_Block:
        case kIROp_SPIRVAsm:
            for (auto child : inst->getChildren())
            {
                addToWorkList({child, entryPoint, parentFunc});
            }
            break;
        case kIROp_SPIRVAsmOperandInst:
            {
                auto operand = as<IRSPIRVAsmOperandInst>(inst);
                addToWorkList({operand->getValue(), entryPoint, parentFunc});
            }
            break;
        }
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            switch (operand->getOp())
            {
            case kIROp_GlobalParam:
            case kIROp_GlobalVar:
            case kIROp_SPIRVAsmOperandBuiltinVar:
                addToWorkList({operand, entryPoint, parentFunc});
                break;
            }
        }
    };

    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->getOp() == kIROp_Func &&
            globalInst->findDecoration<IREntryPointDecoration>())
        {
            auto entryPointFunc = as<IRFunc>(globalInst);
            visit(globalInst, entryPointFunc, nullptr);
        }
    }
    for (Index i = 0; i < workList.getCount(); i++)
        visit(workList[i].inst, workList[i].entryPoint, workList[i].parentFunc);
}

const HashSet<IRFunc*>* CallGraph::getReferencingEntryPoints(IRInst* inst) const
{
    const auto* referencingEntryPoints = m_referencingEntryPoints.tryGetValue(inst);
    if (!referencingEntryPoints)
        return nullptr;
    return referencingEntryPoints;
}

const HashSet<IRFunc*>* CallGraph::getReferencingFunctions(IRInst* inst) const
{
    const auto* referencingFunctions = m_referencingFunctions.tryGetValue(inst);
    if (!referencingFunctions)
        return nullptr;
    return referencingFunctions;
}

const HashSet<IRCall*>* CallGraph::getReferencingCalls(IRFunc* func) const
{
    const auto* referencingCalls = m_referencingCalls.tryGetValue(func);
    if (!referencingCalls)
        return nullptr;
    return referencingCalls;
}

const Dictionary<IRInst*, HashSet<IRFunc*>>& CallGraph::getReferencingEntryPointsMap() const
{
    return m_referencingEntryPoints;
}

} // namespace Slang
