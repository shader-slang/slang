// ir-bind-existentials.cpp
#include "ir-bind-existentials.h"

#include "ir.h"
#include "ir-insts.h"

namespace Slang
{

// The code that comes out of the linking step will have instructions added
// that indicate how parameters with existential (interface) types are supposed
// to be specialized to concrete types.
//
// If there are any global existential-type parameters there should be a
// `bindGlobalExistentialSlots(...)` instruction at module scope.
//
// For each entry point with entry-point existential parameters, there should
// be a `[bindExistentialSlots(...)]` decoration attached to the entry
// point itself.
//
// In each case, the operands of the instruction should be a sequence of
// pairs. The number of pairs should match the number of existential "slots"
// at global or entry-point scope. Each pair should comprise a type `T`
// to plug into the slot, and a witness table `w` for the conformance of
// `T` to the interface type in that slot.
//
// In the simplest case, if we have a global shader parameter of interface
// type:
//
//      IFoo p;
//
// Then this will lower to the IR as:
//
//      global_param p : IFoo;
//
// And if the user tries to specialie `p` to type `Bar`, and a witness
// table `bar_is_ifoo`, we've have:
//
//      bindGlobalExistentialSlots(Bar, bar_is_ifoo);
//
// The goal of this pass is to replace the parameter of interface type
// with one of concrete type:
//
//      global_param p_new : Bar;
//
// and replace any reference to the old `p` parameter with
// a `makeExistential(p_new, bar_is_ifoo)`. That preserves the
// fact that a reference to `p` is conceptually of type `IFoo`,
// but allows downstream optimization passes to start specializing
// code based on the concrete knowledge that the value "backing"
// the parameter is actaully of type `Bar`.

// As is typically for IR passes, we will encapsulate all the
// logic in a `struct` type.
//
struct BindExistentialSlots
{
    IRModule*       module = nullptr;
    DiagnosticSink* sink = nullptr;

    void processModule()
    {
        // We will start by dealing with the global existential slots.
        processGlobalExistentialSlots();

        // Then we will process the per-entry-point existential slots.
        processEntryPointExistentialSlots();
    }

    void processGlobalExistentialSlots()
    {
        // If there are any global existential slots, we will expect
        // to find a `bindGlobalExistentialSlots` instruction at module scope.
        //
        // We will start out by finding that instruction, if it exists.
        //
        IRInst* bindGlobalExistentialSlotsInst = nullptr;
        for( auto inst : module->getGlobalInsts() )
        {
            if( inst->op == kIROp_BindGlobalExistentialSlots )
            {
                bindGlobalExistentialSlotsInst = inst;
                break;
            }
        }

        // Now we will start looking for global shader parameters that make
        // use of existential slots (we can determine this from their
        // layout).
        //
        for( auto inst : module->getGlobalInsts() )
        {
            // We only care about global shader parameters.
            //
            auto globalParam = as<IRGlobalParam>(inst);
            if(!globalParam)
                continue;

            // We will delegate to a subroutine for the meat
            // of the work, since much of it can be shared
            // with the case for entry-point existential
            // parameters.
            //
            processParameter(globalParam, bindGlobalExistentialSlotsInst);
        }

        // Once we are done looping over global shader parameters,
        // all of the relevant information from the
        // `bindGlobalExistentialSlots` instruction will have
        // been moved to the parameters themselves, so we
        // can eliminate the binding instruction.
        //
        if( bindGlobalExistentialSlotsInst )
        {
            bindGlobalExistentialSlotsInst->removeAndDeallocate();
        }
    }

    void processEntryPointExistentialSlots()
    {
        // The overall flow for the entry-point case is similar
        // to the global case.
        //
        // We start by iterating over all the functions at
        // global scope and look for entry points.
        //
        for( auto inst : module->getGlobalInsts() )
        {
            auto func = as<IRFunc>(inst);
            if(!func)
                continue;

            if(!func->findDecorationImpl(kIROp_EntryPointDecoration))
                continue;

            // We then process each entry point we find.
            //
            processEntryPointExistentialSlots(func);
        }
    }

    void processEntryPointExistentialSlots(IRFunc* func)
    {
        // When looking at a single `func`, we need
        // to find the `[bindExistentialSlots(...)]` decoration,
        // if it has one.
        //
        auto bindEntryPointExistentialSlotsInst = func->findDecorationImpl(kIROp_BindExistentialSlotsDecoration);

        // We then need to process each of the entry-point
        // parameters just like we did for global parameters.
        //
        for( auto param : func->getParams() )
        {
            processParameter(param, bindEntryPointExistentialSlotsInst);
        }

        // TODO: We would need to consider what to do if
        // we had an existential return type for `func`.
        //
        // In general, it probably doesn't make sense to
        // have existential types in varying input/output
        // at all, so the front-end should probably be
        // validating that.

        // Once we've processed all the parameters, the information
        // in the `[bindExistentialSlots(...)]` decoration is
        // no longer needed, and we can remove it.
        //
        if( bindEntryPointExistentialSlotsInst )
        {
            bindEntryPointExistentialSlotsInst->removeAndDeallocate();
        }
    }

    // When processing a single parameter we need to have access
    // to the corresponding instruction that will bind its slots.
    //
    // We don't care whether we have a `global_param` and a
    // `bindGlobalExistentialSlots` instruction, or an entry-point
    // function `param` and a `[bindExistentialSlots(...)]`
    // decoration; both use the same subroutine.
    //
    void processParameter(
        IRInst*     param,
        IRInst*     bindSlotsInst)
    {
        // We expect all shader parameters to have layout information,
        // but to be defensive we will skip any that don't.
        //
        auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
        if(!layoutDecoration)
            return;
        auto varLayout = as<VarLayout>(layoutDecoration->getLayout());
        if(!varLayout)
            return;

        // We only care about parameters that are associated
        // with one or more existential slots.
        //
        auto resInfo = varLayout->FindResourceInfo(LayoutResourceKind::ExistentialSlot);
        if(!resInfo)
            return;

        // We will use the layout information on the variable to
        // find out the stating slot, and the information on
        // the type to find out the number of slots.
        //
        UInt firstSlot = resInfo->index;
        UInt slotCount = 0;
        if(auto typeResInfo = varLayout->getTypeLayout()->FindResourceInfo(LayoutResourceKind::ExistentialSlot))
            slotCount = UInt(typeResInfo->count.getFiniteValue());

        // At this point we know that the parameter consumes
        // some number of slots, so it would be an error
        // if we don't have an instruction to bind the slots.
        //
        if( !bindSlotsInst )
        {
            // Note: This error is considered an internal error because
            // we should be detecting and diagnosing this problem before
            // we make it to back-end code generation.
            //
            sink->diagnose(param->sourceLoc, Diagnostics::missingExistentialBindingsForParameter);
            return;
        }

        // Each existential slot corresponds to *two* arguments
        // on the binding instruction: one for the type, and
        // another for the witness table.
        //
        // We will check to make sure we have enough operands to cover
        // this parameter.
        //
        UInt bindOperandCount = bindSlotsInst->getOperandCount();
        if( 2*(firstSlot + slotCount) > bindOperandCount )
        {
            sink->diagnose(param->sourceLoc, Diagnostics::missingExistentialBindingsForParameter);
            return;
        }
        //
        // If there are enough operands, then we will offset to
        // get to the starting point for the current parameter,
        // keeping in mind that each slot accounts for two
        // operands.
        //
        auto operandsForInst = bindSlotsInst->getOperands() + firstSlot;

        // Once we've found the operands that are relevent to
        // the slots used by `param`, we will defer to a routine
        // that replaces the type of `param` based on the
        // information in the slots.
        //
        replaceTypeUsingExistentialSlots(
            param,
            slotCount,
            operandsForInst);
    }

    void replaceTypeUsingExistentialSlots(
        IRInst*         inst,
        UInt            slotCount,
        IRUse const*    slotArgs)
    {
        SLANG_UNUSED(slotCount);

        // We are going to alter the type of the
        // given `inst` based on information in
        // the `slotArgs`, but the exact kind
        // of modification will depend on the
        // original type of `inst`.

        auto fullType = inst->getFullType();
        auto type = inst->getDataType();

        SharedIRBuilder sharedBuilder;
        sharedBuilder.session = module->getSession();
        sharedBuilder.module = module;

        IRBuilder builder;
        builder.sharedBuilder = &sharedBuilder;

        // The easy case is when the `type` of `inst`
        // is directly an interface type.
        //
        if( auto interfaceType = as<IRInterfaceType>(type) )
        {
            // An intereface-type parameter will use a
            // single slot, which consits of a pair of
            // operands.
            //
            // The first operand is the concrete type
            // we want to plug in.
            //
            auto newType = (IRType*) slotArgs[0].get();

            // The second operand is a witness that
            // the concrete type conforms to the interface
            // used for the original parameter.
            //
            auto newWitnessTable = slotArgs[1].get();

            // We are going to replace the (interface) type of
            // the parameter with the new (concrete) type.
            //
            builder.setDataType(inst, newType);

            // Next we want to replace all uses of `inst` (which
            // expect a value of its old type) with a fresh
            // `makeExistential(...)` instruction that refers to
            // `inst` with its new type.
            //
            // Note: we make a copy of the list of uses for `inst`
            // before going through and replacing them, because
            // during the replacement we make *more* uses of `inst`,
            // as an operand to the `makeExistential` instructions.
            // We only want to replace the old uses, and not the
            // new ones we'll be making.
            //
            List<IRUse*> usesToReplace;
            for(auto use = inst->firstUse; use; use = use->nextUse )
                usesToReplace.Add(use);

            // Now we can loop over our list of uses and replace each.
            //
            for(auto use : usesToReplace)
            {
                // First we emit a `makeExisential` right before the
                // use site.
                //
                builder.setInsertBefore(use->getUser());
                auto newVal = builder.emitMakeExistential(
                    fullType,
                    inst,
                    newWitnessTable);

                // Second we make the use site point at the new
                // value instead.
                //
                use->set(newVal);
            }
        }
        else
        {
            // TODO: We eventually need to handle cases where there
            // are:
            //
            // * Arrays over existential types; e.g.: `IFoo[3]`
            //
            // * Structs with existential-type fields.
            //
            // * Constant buffers or other "containers" over existentials; e.g., `ConstantBuffer<IFoo>`
            //
            // * Nested combinations of the above; e.g., a `ConstantBuffer`
            //   of a struct with a field that is an array of `IFoo`.
            //
            SLANG_UNIMPLEMENTED_X("shader parameters with nested existentials");
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
