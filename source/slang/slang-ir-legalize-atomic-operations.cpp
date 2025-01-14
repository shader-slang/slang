#include "slang-ir-legalize-atomic-operations.h"

#include "slang-ir-insts.h"

namespace Slang
{

// Returns whether 'dst' is a valid destination for atomic operations, meaning
// it leads either to 'groupshared' or 'device buffer' memory.
static bool isValidAtomicDest(IRInst* dst)
{
    bool isGroupShared = as<IRGroupSharedRate>(dst->getRate());
    if (isGroupShared)
        return true;

    if (as<IRRWStructuredBufferGetElementPtr>(dst))
        return true;
    if (as<IRGlobalParam>(dst))
    {
        switch (dst->getDataType()->getOp())
        {
        case kIROp_GLSLShaderStorageBufferType:
        case kIROp_TextureType:
            return true;
        default:
            return false;
        }
    }

    if (auto load = as<IRLoad>(dst))
        return isValidAtomicDest(load->getAddress());
    if (auto imageSubScript = as<IRImageSubscript>(dst))
        return isValidAtomicDest(imageSubScript->getImage());
    if (auto getElementPtr = as<IRGetElementPtr>(dst))
        return isValidAtomicDest(getElementPtr->getBase());
    if (auto getOffsetPtr = as<IRGetOffsetPtr>(dst))
        return isValidAtomicDest(getOffsetPtr->getBase());
    if (auto fieldAddress = as<IRFieldAddress>(dst))
        return isValidAtomicDest(fieldAddress->getBase());

    return false;
}

static void validateAtomicOperations(DiagnosticSink* sink, IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_AtomicLoad:
    case kIROp_AtomicStore:
    case kIROp_AtomicExchange:
    case kIROp_AtomicCompareExchange:
    case kIROp_AtomicAdd:
    case kIROp_AtomicSub:
    case kIROp_AtomicAnd:
    case kIROp_AtomicOr:
    case kIROp_AtomicXor:
    case kIROp_AtomicMin:
    case kIROp_AtomicMax:
    case kIROp_AtomicInc:
    case kIROp_AtomicDec:
        {
            IRInst* destinationPtr = inst->getOperand(0);
            if (!isValidAtomicDest(destinationPtr))
                sink->diagnose(inst->sourceLoc, Diagnostics::invalidAtomicDestinationPointer);
        }
        break;

    default:
        break;
    }

    for (auto child : inst->getModifiableChildren())
    {
        validateAtomicOperations(sink, child);
    }
}

void legalizeAtomicOperations(DiagnosticSink * sink, IRModule* module)
{
	validateAtomicOperations(sink, module->getModuleInst());
}
} // namespace Slang
