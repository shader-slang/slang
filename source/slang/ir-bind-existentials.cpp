// ir-bind-existentials.cpp
#include "ir-bind-existentials.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang
{

struct BindExistentialSlots
{
    IRModule*       module = nullptr;
    DiagnosticSink* sink = nullptr;

    IRInst* bindGlobalExistentialSlotsInst = nullptr;

    void processModule()
    {
        // TODO: Need to figure out what to do in this step...

        // First, let's find the `bindGlobalExistentialSlots` instruction, if we have one.
        for( auto inst : module->getGlobalInsts() )
        {
            if( inst->op == kIROp_BindGlobalExistentialSlots )
            {
                bindGlobalExistentialSlotsInst = inst;
                break;
            }
        }

        // Note: It isn't an error if we don't have a `bindGlobalExistentialSlots` instruction,
        // because there might not be any global shader parameters that needed it.
        //
        // No matter what, we want to scan for global shader parameters, and see (using their
        // layout) whether they require any of the global existential slots to help fill in
        // their type.
        //
        for( auto inst : module->getGlobalInsts() )
        {
            auto globalParam = as<IRGlobalParam>(inst);
            if(!globalParam)
                continue;

            auto layoutDecoration = globalParam->findDecoration<IRLayoutDecoration>();
            if(!layoutDecoration)
                continue;

            auto varLayout = as<VarLayout>(layoutDecoration->getLayout());
            if(!varLayout)
                continue;

            auto resInfo = varLayout->FindResourceInfo(LayoutResourceKind::ExistentialSlot);
            if(!resInfo)
                continue;

            UInt firstSlot = resInfo->index;
            UInt slotCount = 0;
            if(auto typeResInfo = varLayout->getTypeLayout()->FindResourceInfo(LayoutResourceKind::ExistentialSlot))
                slotCount = UInt(typeResInfo->count.getFiniteValue());

            // Okay, let's check if we have enough arguments.

            if( !bindGlobalExistentialSlotsInst )
            {
                sink->diagnose(inst->sourceLoc, Diagnostics::unexpected, "no global existential bindings");
                break;
            }

            UInt slotArgCount = bindGlobalExistentialSlotsInst->getOperandCount();
            if( (firstSlot + slotCount) > slotArgCount )
            {
                sink->diagnose(inst->sourceLoc, Diagnostics::unexpected, "not enough global existential args");
                break;
            }

            // Okay, we have the parameter, and now we want to silently replace
            // its type with one that uses the types from the existential slot args...

            auto operandsForInst = bindGlobalExistentialSlotsInst->getOperands() + firstSlot;
            replaceTypeUsingExistentialSlots(
                globalParam,
                slotCount,
                operandsForInst);

#if 0

            // Okay, we can find just the operands we need.
            List<IRInst*> argsForInst;
            for( UInt ii = 0; ii < slotCount; ++ii )
            {
                auto arg = bindGlobalExistentialSlotsInst->getOperand(firstSlot + ii);
                argsForInst.Add(arg);
            }

            SharedIRBuilder sharedBuilder;
            sharedBuilder.session = module->getSession();
            sharedBuilder.module = module;

            IRBuilder builder;
            builder.sharedBuilder = &sharedBuilder;

            // Move the decoration down to the parameter itself, so that we can manipulate it independently.
            builder.addBindExistentialSlotsDecoration(globalParam, argsForInst.Count(), argsForInst.Buffer());
#endif
        }

        if( bindGlobalExistentialSlotsInst )
        {
            bindGlobalExistentialSlotsInst->removeAndDeallocate();
        }

        // TODO: need to process entry-point parameter lists too...

        // Next we will look for any entry points, and deal with existential slot bindings for them too

        for( auto inst : module->getGlobalInsts() )
        {
            auto func = as<IRFunc>(inst);
            if(!func)
                continue;

            if(!func->findDecorationImpl(kIROp_EntryPointDecoration))
                continue;

            auto bindEntryPointExistentialSlotsInst = func->findDecorationImpl(kIROp_BindExistentialSlotsDecoration);

            for( auto param : func->getParams() )
            {
                auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
                if(!layoutDecoration)
                    continue;

                auto varLayout = as<VarLayout>(layoutDecoration->getLayout());
                if(!varLayout)
                    continue;

                auto resInfo = varLayout->FindResourceInfo(LayoutResourceKind::ExistentialSlot);
                if(!resInfo)
                        continue;

                UInt firstSlot = resInfo->index;
                UInt slotCount = 0;
                if(auto typeResInfo = varLayout->getTypeLayout()->FindResourceInfo(LayoutResourceKind::ExistentialSlot))
                    slotCount = UInt(typeResInfo->count.getFiniteValue());

                // Okay, let's check if we have enough arguments.

                if( !bindEntryPointExistentialSlotsInst )
                {
                    sink->diagnose(inst->sourceLoc, Diagnostics::unexpected, "no entry point existential bindings");
                    break;
                }

                auto operandsForInst = bindEntryPointExistentialSlotsInst->getOperands() + firstSlot;
                replaceTypeUsingExistentialSlots(
                    param,
                    slotCount,
                    operandsForInst);

#if 0
                UInt slotArgCount = bindEntryPointExistentialSlotsInst->getOperandCount();
                if( (firstSlot + slotCount) > slotArgCount )
                {
                    sink->diagnose(inst->sourceLoc, Diagnostics::unexpected, "not enough global existential args");
                    break;
                }

                // Okay, we can find just the operands we need.
                List<IRInst*> argsForInst;
                for( UInt ii = 0; ii < slotCount; ++ii )
                {
                    auto arg = bindEntryPointExistentialSlotsInst->getOperand(firstSlot + ii);
                    argsForInst.Add(arg);
                }

                SharedIRBuilder sharedBuilder;
                sharedBuilder.session = module->getSession();
                sharedBuilder.module = module;

                IRBuilder builder;
                builder.sharedBuilder = &sharedBuilder;

                // Move the decoration down to the parameter itself, so that we can manipulate it independently.
                builder.addBindExistentialSlotsDecoration(param, argsForInst.Count(), argsForInst.Buffer());
#endif
            }

            if( bindEntryPointExistentialSlotsInst )
            {
                bindEntryPointExistentialSlotsInst->removeAndDeallocate();
            }
        }

        // Okay, now all the decorations are on the individual parameters... great...


    }

    void replaceTypeUsingExistentialSlots(
        IRInst*         inst,
        UInt            slotCount,
        IRUse const*    slotArgs)
    {
        SLANG_UNUSED(slotCount);

        auto fullType = inst->getFullType();
        auto type = inst->getDataType();

        SharedIRBuilder sharedBuilder;
        sharedBuilder.session = module->getSession();
        sharedBuilder.module = module;

        IRBuilder builder;
        builder.sharedBuilder = &sharedBuilder;

        if( auto interfaceType = as<IRInterfaceType>(type) )
        {
            // This is the easy case...

            auto newType = (IRType*) slotArgs[0].get();
            auto newWitnessTable = slotArgs[1].get();

            // Okay, we can do this.

            builder.setDataType(inst, newType);

            // Note: we make a copy of the list of uses for `inst`
            // before going through and replacing them, because
            // during the replacement we make *more* uses of `inst`,
            // as an operand to `makeExistential` instructions.
            // We only want to replace the old uses, and not the
            // new ones we'll be making.
            //
            List<IRUse*> usesToReplace;
            for(auto use = inst->firstUse; use; use = use->nextUse )
                usesToReplace.Add(use);

            for(auto use : usesToReplace)
            {
                builder.setInsertBefore(use->getUser());

                auto newVal = builder.emitMakeExistential(
                    fullType,
                    inst,
                    newWitnessTable);

                use->set(newVal);
            }
        }
        else
        {
            SLANG_UNEXPECTED("unexpected type when replacing existential slots");
        }
    }
};

void bindExistentialSlots(
    IRModule*       module,
    DiagnosticSink* sink)
{
    BindExistentialSlots context;
    context.module = module;
    context.sink = sink;
    context.processModule();
}

}
