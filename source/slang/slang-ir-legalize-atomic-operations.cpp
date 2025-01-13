#include "slang-ir-legalize-atomic-operations.h"

#include "slang-ir-insts.h"

namespace Slang
{

static void validateAtomicOperations(IRInst * inst)
{
	switch (inst->getOp())
	{
	case kIROp_AtomicAdd:
		// TODO: validate the destination pointer
		break;
    // TODO: Handle rest of kIROp_Atomic*

    case kIROp_SPIRVAsmInst:
    {
        auto spvAsmInst = as<IRSPIRVAsmInst>(inst);
        auto op0 = spvAsmInst->getOperand(0);
        if (op0->getOp() == kIROp_SPIRVAsmOperandEnum)
        {
            auto opInt = as<IRIntLit>(op0->getOperand(0));
            auto opIntVal = opInt->getValue();
            if (opIntVal == 234) // OpAtomicIAdd
            {
                // TODO: validate the destination pointer
            }
            // TODO: Other SPIR-V atomic ops...
        }
    }
    break;

	default:
		for (auto child : inst->getModifiableChildren())
		{
			validateAtomicOperations(child);
		}
		break;
	}
}	

void legalizeAtomicOperations(IRModule* module)
{
	validateAtomicOperations(module->getModuleInst());
}
} // namespace Slang
