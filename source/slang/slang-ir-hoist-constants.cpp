// slang-ir-hoist-constants.cpp
#include "slang-ir-hoist-constants.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

struct HoistConstantPass : InstPassBase
{
    HoistConstantPass(IRModule* module) : InstPassBase(module)
    {}

    bool changed = false;

    void processModule()
    {
        sharedBuilderStorage.init(module);

        processAllInsts([this](IRInst* inst)
            {
                
                if (inst->getParent() == module->getModuleInst() || !inst->getParent())
                    return;
                auto parent = inst->getParent();
                auto p = parent;
                while (p)
                {
                    if (as<IRGlobalValueWithCode>(p))
                        return;
                    p = p->parent;
                }
                while (parent && parent->parent != module->getModuleInst())
                    parent = parent->parent;
                if (!parent)
                    return;
                switch (inst->getOp())
                {
                default:
                    return;
                case kIROp_Add:
                case kIROp_Sub:
                case kIROp_Mul:
                case kIROp_Div:
                case kIROp_Module:
                case kIROp_Neg:
                case kIROp_And:
                case kIROp_Or:
                case kIROp_Not:
                case kIROp_BitAnd:
                case kIROp_BitNot:
                case kIROp_BitOr:
                case kIROp_BitXor:
                case kIROp_Select:
                case kIROp_Greater:
                case kIROp_Less:
                case kIROp_Leq:
                case kIROp_Geq:
                case kIROp_Eql:
                case kIROp_Neq:
                case kIROp_BitCast:
                case kIROp_Lsh:
                case kIROp_Rsh:
                case kIROp_Construct:
                case kIROp_makeVector:
                case kIROp_MakeMatrix:
                case kIROp_swizzle:
                case kIROp_IntLit:
                case kIROp_BoolLit:
                case kIROp_ArrayType:
                case kIROp_Specialize:
                case kIROp_VectorType:
                    break;
                }
                if (inst->typeUse.get() && inst->typeUse.get()->parent != module->getModuleInst())
                    return;
                for (UInt i = 0; i < inst->getOperandCount(); i++)
                {
                    if (inst->getOperand(i)->parent != module->getModuleInst())
                        return;
                }
                // all operands are in global scope, we can move this inst to global scope as well.
                inst->insertBefore(parent);
                changed = true;
            });
    }
};

bool hoistConstants(
    IRModule* module)
{
    HoistConstantPass context(module);
    context.processModule();
    return context.changed;
}

}
