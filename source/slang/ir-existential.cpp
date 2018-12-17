// ir-existential.cpp
#include "ir-existential.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang {

struct ExistentialTypeSimplificationContext
{
    List<IRInst*> instsToRemove;

};

void simplifyExistentialTypesRec(
    ExistentialTypeSimplificationContext*   context,
    IRInst*                                 inst)
{
    switch( inst->op )
    {
    default:
        break;

    case kIROp_ExtractExistentialValue:
        {
            auto arg = inst->getOperand(0);
            if( auto makeExistential = as<IRMakeExistential>(arg) )
            {
                auto value = makeExistential->getWrappedValue();
                inst->replaceUsesWith(value);
                context->instsToRemove.Add(inst);
            }
        }
        break;

    case kIROp_ExtractExistentialType:
        {
            auto arg = inst->getOperand(0);
            if( auto makeExistential = as<IRMakeExistential>(arg) )
            {
                auto value = makeExistential->getWrappedValue();
                inst->replaceUsesWith(value->getFullType());
                context->instsToRemove.Add(inst);
            }
        }
        break;

    case kIROp_ExtractExistentialWitnessTable:
        {
            auto arg = inst->getOperand(0);
            if( auto makeExistential = as<IRMakeExistential>(arg) )
            {
                auto witnessTable = makeExistential->getWitnessTable();
                inst->replaceUsesWith(witnessTable);
                context->instsToRemove.Add(inst);
            }
        }
        break;
    }

    for( auto childInst : inst->getChildren() )
    {
        simplifyExistentialTypesRec(context, childInst);
    }
}

void removeUnusedExistentialsRec(
    ExistentialTypeSimplificationContext*   context,
    IRInst*                                 inst)
{
    switch( inst->op )
    {
    default:
        break;

    case kIROp_MakeExistential:
        {
            if( !inst->hasUses() )
            {
                context->instsToRemove.Add(inst);
            }
        }
        break;
    }

    for( auto childInst : inst->getChildren() )
    {
        removeUnusedExistentialsRec(context, childInst);
    }
}

void simplifyExistentialTypes(
    IRModule*   module)
{
    {
        ExistentialTypeSimplificationContext context;
        simplifyExistentialTypesRec(&context, module->getModuleInst());
        for( auto inst : context.instsToRemove )
        {
            inst->removeAndDeallocate();
        }
    }

    {
        ExistentialTypeSimplificationContext context;
        removeUnusedExistentialsRec(&context, module->getModuleInst());
        for( auto inst : context.instsToRemove )
        {
            inst->removeAndDeallocate();
        }
    }
}

} // namespace Slang
