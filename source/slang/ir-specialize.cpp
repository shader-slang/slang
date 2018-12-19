// ir-specialize.cpp
#include "ir-specialize.h"

#include "ir.h"
#include "ir-clone.h"
#include "ir-insts.h"

namespace Slang
{

// This file implements the primary specialization pass, that takes
// generic/polymorphic Slang code and specializes/monomorphises it.
//
// At present this primarily means generating specialized copies
// of generic functions/types based on the concrete types used
// at specialization sites, and also specializing instances
// of witness-table lookup to directly refer to the concrete
// values for witnesses when witness tables are known.
//
// Eventually, this pass will also need to perform specialization
// of functions to argument values for parameters that must
// be compile-time constants, and simplification of code using
// existential (interface) types for function parameters/results.

struct SpecializationContext
{
    // We know that we can only perform specialization when all
    // of the arguments to a generic are also fully specialized.
    // The "is fully specialized" condition is something we
    // need to solve for over the program, because the fully-
    // specialized-ness of an instruction depends on the
    // fully-specialized-ness of its operands.
    //
    // We will build an explicit hash set to encode those
    // instructions that are fully specialized.
    //
    HashSet<IRInst*> fullySpecializedInsts;

    // An instruction is then fully specialized if and only
    // if it is in our set.
    //
    bool isInstFullySpecialized(
        IRInst*                     inst)
    {
        // A small wrinkle is that a null instruction pointer
        // sometimes appears a a type, and so should be treated
        // as fully specialized too.
        //
        // TODO: It would be nice to remove this wrinkle.
        //
        if(!inst) return true;

        return fullySpecializedInsts.Contains(inst);
    }

    // When an instruction isn't fully specialized, but its operands *are*
    // then it is a candidate for specialization itself, so we will have
    // a query to check for the "all operands fully specialized" case.
    //
    bool areAllOperandsFullySpecialized(
        IRInst*                     inst)
    {
        if(!isInstFullySpecialized(inst->getFullType()))
            return false;

        UInt operandCount = inst->getOperandCount();
        for(UInt ii = 0; ii < operandCount; ++ii)
        {
            IRInst* operand = inst->getOperand(ii);
            if(!isInstFullySpecialized(operand))
                return false;
        }

        return true;
    }

    // We will also maintain a work list of instructions that are
    // not fully specialized, and that we want to consider for
    // specialization.
    //
    List<IRInst*> workList;

    // When we consider adding an instruction to our work list
    // we will try to be careful and only add things that aren't
    // already fully specialized.
    //
    void maybeAddToWorkList(IRInst* inst)
    {
        if(isInstFullySpecialized(inst))
            return;

        workList.Add(inst);
    }

    // When we go to populate the work list by recursively
    // traversing some code, we will be careful to *not*
    // add generics or their children to the work list,
    // and will instead consider a generic to be "fully
    // specialized" already (because uses of that generic
    // as an *operand* should be seen as fully specialized
    // references).
    //
    void populateWorkListRec(
        IRInst*                     inst)
    {
        if(auto genericInst = as<IRGeneric>(inst))
        {
            fullySpecializedInsts.Add(genericInst);
        }
        else
        {
            maybeAddToWorkList(inst);

            for(auto child : inst->getChildren())
            {
                populateWorkListRec(child);
            }
        }
    }

    // Of course, somewhere along the way we expect
    // to run into uses of `specialize(...)` instructions
    // to bind a generic to arguments that we want to
    // specialize into concrete code.
    //
    // We also know that if we encouter `specialize(g, a, b, c)`
    // and then later `specialize(g, a, b, c)` again, we
    // only want to generate the specialized code for `g<a,b,c>`
    // *once*, and re-use it for both versions.
    //
    // We will cache existing specializations of generic function/types
    // using the simple key type defined as part of the IR cloning infrastructure.
    //
    typedef IRSimpleSpecializationKey Key;
    Dictionary<Key, IRInst*> genericSpecializations;

    // We will also use some shared IR building state across
    // all of our specialization/cloning steps.
    //
    SharedIRBuilder sharedBuilderStorage;

    // Now let's look at the task of finding or generation a
    // specialization of some generic `g`, given a specialization
    // instruction like `specialize(g, a, b, c)`.
    //
    // The `specializeGeneric` function will return a value
    // suitable for use as a replacement for the `specialize(...)`
    // instruction.
    //
    IRInst* specializeGeneric(
        IRGeneric*      genericVal,
        IRSpecialize*   specializeInst)
    {
        // First, we want to see if an existing specialization
        // has already been made. To do that we will construct a key
        // for lookup in the generic specialization context.
        //
        // Our key will consist of the identity of the generic
        // being specialized, and each of the argument values
        // being pased to it. In our hypothetical example of
        // `specialize(g, a, b, c)` the key will then be
        // the array `[g, a, b, c]`.
        //
        Key key;
        key.vals.Add(specializeInst->getBase());
        UInt argCount = specializeInst->getArgCount();
        for( UInt ii = 0; ii < argCount; ++ii )
        {
            key.vals.Add(specializeInst->getArg(ii));
        }

        {
            // We use our generated key to look for an
            // existing specialization that has been registered.
            // If one is found, our work is done.
            //
            IRInst* specializedVal = nullptr;
            if(genericSpecializations.TryGetValue(key, specializedVal))
                return specializedVal;
        }

        // If no existing specialization is found, we need
        // to create the specialization instead.
        //
        // Effectively this amounts to "calling" the generic
        // on its concrete argument values and computing the
        // result it returns.
        //
        // For now, all of our generics consist of a single
        // basic block, so we can "call" them just by
        // cloning the instructions in their single block
        // into the global scope, using an environment for
        // cloning that maps the generic parameters to
        // the concrete arguments that were provided
        // by the `specialize(...)` instruction.
        //
        IRCloneEnv      env;

        // We will walk through the parameters of the generic and
        // register the corresponding argument of the `specialize`
        // instruction to be used as the "cloned" value for each
        // parameter.
        //
        // Suppose we are looking at `specialize(g, a, b, c)` and `g` has
        // three generic parameters: `T`, `U`, and `V`. Then we will
        // be initializing our environment to map `T -> a`, `U -> b`,
        // and `V -> c`.
        //
        UInt argCounter = 0;
        for( auto param : genericVal->getParams() )
        {
            UInt argIndex = argCounter++;
            SLANG_ASSERT(argIndex < specializeInst->getArgCount());

            IRInst* arg = specializeInst->getArg(argIndex);

            env.mapOldValToNew.Add(param, arg);
        }

        // We will set up an IR builder for insertion
        // into the global scope, at the same location
        // as the original generic.
        //
        IRBuilder builderStorage;
        IRBuilder* builder = &builderStorage;
        builder->sharedBuilder = &sharedBuilderStorage;
        builder->setInsertBefore(genericVal);

        // Now we will run through the body of the generic and
        // clone each of its instructions into the global scope,
        // until we reach a `return` instruction.
        //
        for( auto bb : genericVal->getBlocks() )
        {
            // We expect a generic to only ever contain a single block.
            //
            SLANG_ASSERT(bb == genericVal->getFirstBlock());

            // We will iterate over the non-parameter ("ordinary")
            // instructions only, because parameters were dealt
            // with explictly at an earlier point.
            //
            for( auto ii : bb->getOrdinaryInsts() )
            {
                // The last block of the generic is expected to end with
                // a `return` instruction for the specialized value that
                // comes out of the abstraction.
                //
                // We thus use that cloned value as the result of the
                // specialization step.
                //
                if( auto returnValInst = as<IRReturnVal>(ii) )
                {
                    auto specializedVal = findCloneForOperand(&env, returnValInst->getVal());

                    // The value that was returned from evaluating
                    // the generic is the specialized value, and we
                    // need to remember it in our dictionary of
                    // specializations so that we don't instantiate
                    // this generic again for the same arguments.
                    //
                    genericSpecializations.Add(key, specializedVal);

                    return specializedVal;
                }

                // For any instruction other than a `return`, we will
                // simply clone it completely into the global scope.
                //
                IRInst* clonedInst = cloneInst(&env, builder, ii);

                // Any new instructions we create during cloning were
                // not present when we initially built our work list,
                // so we need to make sure to consider them now.
                //
                // This is important for the cases where one generic
                // invokes another, because there will be `specialize`
                // operations nested inside the first generic that refer
                // to the second.
                //
                populateWorkListRec(clonedInst);
            }
        }

        // If we reach this point, something went wrong, because we
        // never encountered a `return` inside the body of the generic.
        //
        SLANG_UNEXPECTED("no return from generic");
        UNREACHABLE_RETURN(nullptr);
    }

    // The logic for generating a specialization of an IR generic
    // relies on the ability to "evaluate" the code in the body of
    // the generic, but that obviously doesn't work if we don't
    // actually have the full definition for the body.
    //
    // This can arise in particular for builtin operations/types.
    //
    // Before calling `specializeGeneric()` we need to make sure
    // that the generic is actually amenable to specialization,
    // by looking at whether it is a definition or a declaration.
    //
    bool canSpecializeGeneric(
        IRGeneric*  generic)
    {
        // It is possible to have multiple "layers" of generics
        // (e.g., when a generic function is nested in a generic
        // type). Therefore we need to drill down through all
        // of the layers present to see if at the leaf we have
        // something that looks like a definition.
        //
        IRGeneric* g = generic;
        for(;;)
        {
            // Given the generic `g`, we will find the value
            // it appears to return in its body.
            //
            auto val = findGenericReturnVal(g);
            if(!val)
                return false;

            // If `g` returns an inner generic, then we need
            // to drill down further.
            //
            if (auto nestedGeneric = as<IRGeneric>(val))
            {
                g = nestedGeneric;
                continue;
            }

            // Once we've found the leaf value that will be produced
            // after all specialization is complete, we can check
            // whether it looks like a definition or not.
            //
            return isDefinition(val);
        }
    }

    // Now that we know when we can specialize a generic, and how
    // to do it, we can write a subroutine that takes a
    // `specialize(g, a, b, c, ...)` instruction and performs
    // specialization if it is possible.
    //
    void maybeSpecializeGeneric(
        IRSpecialize* specInst)
    {
        // The invariant that the arguments are fully specialized
        // should mean that `a, b, c, ...` are in a form that
        // we can work with, but it does *not* guarantee
        // that the `g` operand is something we can work with.
        //
        // We can only perform specialization in the case where
        // the base `g` is a known `generic` instruction.
        //
        auto baseVal = specInst->getBase();
        auto genericVal = as<IRGeneric>(baseVal);
        if(!genericVal)
            return;

        // We can also only specialize a generic if it
        // represents a definition rather than a declaration.
        //
        if(!canSpecializeGeneric(genericVal))
            return;

        // Once we know that specialization is possible,
        // the actual work is fairly simple.
        //
        // First, we find or generate a specialized
        // version of the result of the generic (a specialized
        // type, function, or whatever).
        //
        auto specializedVal = specializeGeneric(genericVal, specInst);

        // Then we simply replace any uses of the `specialize(...)`
        // instruction with the specialized value and delete
        // the `specialize(...)` instruction from existence.
        //
        specInst->replaceUsesWith(specializedVal);
        specInst->removeAndDeallocate();
    }

    // The basic rule we are following is that once all the operands
    // to an instruction are fully specialized, we are safe
    // to specialize the instruction itself, but the work
    // required to specialize an instruction depends on the
    // form of the instruction.
    //
    void fullySpecializeInst(
        IRInst*                     inst)
    {
        // A precondition of the `fullySpecializeInst` operation
        // is that the operands to `inst` have all been fully
        // specialized.
        //
        SLANG_ASSERT(areAllOperandsFullySpecialized(inst));

        switch(inst->op)
        {
        default:
            // The default case is that there is nothing to
            // be done to specialize an instruction; once all
            // of its operands are specialized it is safe
            // to consider the instruction itself as fully
            // specialized.
            //
            break;

        case kIROp_Specialize:
            // The logic for specializing a `specialize(...)`
            // instruction has already been elaborated above.
            //
            maybeSpecializeGeneric(cast<IRSpecialize>(inst));
            break;

        case kIROp_lookup_interface_method:
            // The remaining case we need to consider here
            // is when we have a `lookup_witness_method` instruction
            // that is being applied to a concrete witness table,
            // because we can specialize it to just be a direct
            // reference to the actual witness value from the table.
            //
            maybeSpecializeWitnessLookup(cast<IRLookupWitnessMethod>(inst));
            break;
        }
    }

    void maybeSpecializeWitnessLookup(
        IRLookupWitnessMethod* lookupInst)
    {
        // Note: While we currently have named the instruction
        // `lookup_witness_method`, the `method` part is a misnomer
        // and the same instruction can look up *any* interface
        // requirement based on the witness table that provides
        // a conformance, and the "key" that indicates the interface
        // requirement.

        // We can only specialize in the case where the lookup
        // is being done on a concrete witness table, and not
        // the result of a `specialize` instruction or other
        // operation that will yield such a table.
        //
        auto witnessTable = as<IRWitnessTable>(lookupInst->getWitnessTable());
        if(!witnessTable)
            return;

        // Because we have a concrete witness table, we can
        // use it to look up the IR value that satisfies
        // the given interface requirement.
        //
        auto requirementKey = lookupInst->getRequirementKey();
        auto satisfyingVal = findWitnessVal(witnessTable, requirementKey);

        // We expect to always find a satisfying value, but
        // we will go ahead and code defensively so that
        // we leave "correct" but unspecialized code if
        // we cannot find a concrete value to use.
        //
        if(!satisfyingVal)
            return;

        // At this point, we know that `satisfyingVal` is what
        // would result from executing this `lookup_witness_method`
        // instruciton dynamically, so we can go ahead and
        // replace the original instruction with that value.
        //
        lookupInst->replaceUsesWith(satisfyingVal);
        lookupInst->removeAndDeallocate();
    }

    // The above subroutine needed a way to look up
    // the satisfying value for a given requirement
    // key in a concrete witness table, so let's
    // define that now.
    //
    IRInst* findWitnessVal(
        IRWitnessTable* witnessTable,
        IRInst*         requirementKey)
    {
        // A witness table is basically just a container
        // for key-value pairs, and so the best we can
        // do for now is a naive linear search.
        //
        for( auto entry : witnessTable->getEntries() )
        {
            if (requirementKey == entry->getRequirementKey())
            {
                return entry->getSatisfyingVal();
            }
        }
        return nullptr;
    }

    // All of the machinery has been defined above, so
    // we can now walk through the flow of the overall
    // specialization pass.
    //
    void processModule(IRModule* module)
    {
        // We start by initializing our shared IR building state,
        // since we will re-use that state for any code we
        // generate along the way.
        //
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->module = module;
        sharedBuilder->session = module->session;

        // The unspecialized IR we receive as input will have
        // `IRBindGlobalGenericParam` instructions that associate
        // each global-scope generic parameter (a type, witness
        // table, or what-have-you) with the value that it should
        // be bound to for the purposes of this code-generation
        // pass.
        //
        // Before doing any other specialization work, we will
        // iterate over these instructions (which may only
        // appear at the global scope) and use them to drive
        // replacement of the given generic type parameter with
        // the desired concrete value.
        //
        auto moduleInst = module->getModuleInst();
        for(auto inst : moduleInst->getChildren())
        {
            // We only want to consider the `bind_global_generic_param`
            // instructions, and ignore everything else.
            //
            auto bindInst = as<IRBindGlobalGenericParam>(inst);
            if(!bindInst)
                continue;

            // HACK: Our current front-end emit logic can end up emitting multiple
            // `bind_global_generic_param` instructions for the same parameter. This is
            // a buggy behavior, but a real fix would require refactoring the way
            // global generic arguments are specified today.
            //
            // For now we will do a sanity check to detect parameters that
            // have already been specialized.
            //
            if( !as<IRGlobalGenericParam>(bindInst->getOperand(0)) )
            {
                // The "parameter" operand is no longer a parameter, so it
                // seems things must have been specialized already.
                //
                continue;
            }

            // The actual logic for applying the substitution is
            // almost trivial: we will replace any uses of the
            // global generic parameter with its desired value.
            //
            auto param = bindInst->getParam();
            auto val = bindInst->getVal();
            param->replaceUsesWith(val);
        }
        {
            // Now that we've replaced any uses of global generic
            // parameters, we will do a second pass to remove
            // the parameters and any `bind_global_generic_param`
            // instructions, since both should be dead/unused.
            //
            IRInst* next = nullptr;
            for(auto inst = moduleInst->getFirstChild(); inst; inst = next)
            {
                next = inst->getNextInst();

                switch(inst->op)
                {
                default:
                    break;

                case kIROp_GlobalGenericParam:
                case kIROp_BindGlobalGenericParam:
                    // A `bind_global_generic_param` instruction should
                    // have no uses in the first place, and all the global
                    // generic parameters should have had their uses replaced.
                    //
                    SLANG_ASSERT(!inst->firstUse);
                    inst->removeAndDeallocate();
                    break;
                }
            }
        }

        // Now that we've eliminated all cases of global generic parameters,
        // we should now have the properties that:
        //
        // 1. Execution starts in non-generic code, with no unbound
        //    generic parameters in scope.
        //
        // 2. Any case where non-generic code makes use of a generic
        //    type/function, there will be a `specialize` instruction
        //    that specifies both the generic and the (concrete) type
        //    arguments that should be provided to it.
        //
        // Our primary goal is then to find `specialize` instructions that
        // can be replaced with references to, e.g., a suitably
        // specialized function, and to resolve any `lookup_interface_method`
        // instructions to the concrete value fetched from a witness
        // table.
        //
        // We need to be careful of a few things:
        //
        // * It would not in general make sense to consider specialize-able
        //   instructions under an `IRGeneric`, since that could mean "specializing"
        //   code to parameter values that are still unknown.
        //
        // * We *also* need to be careful not to specialize something when one
        //   or more of its inputs is also a `specialize` or `lookup_interface_method`
        //   instruction, because then we'd be propagating through non-concrete
        //   values.
        //
        // The approach we use here is to build a work list of instructions
        // that are candidates for specialization, and then process them one
        // at a time to see if we can make some forward progress.
        //
        // We will start by recursively walking all the instructions to add
        // the appropriate ones to  our work list:
        //
        populateWorkListRec(moduleInst);

        // We want to treat our work list like a queue rather than
        // a stack, because we populated it in program order,
        // and fully-specialized-ness will tend to flow top-down.
        //
        // To accomplish this we will ping-pong between the
        // real work list and a copy so that we can iterate over
        // one list while adding to the other.
        //
        List<IRInst*> workListCopy;
        while(workList.Count() != 0)
        {
            workListCopy.Clear();
            workListCopy.SwapWith(workList);

            for( auto inst : workListCopy )
            {
                // We need to check whether it is possible to specialize
                // the instruction yet. It might not be specializable
                // because its operands haven't been specialized.
                //
                if(!areAllOperandsFullySpecialized(inst))
                {
                    // If we can't fully specialize this instruction
                    // yet, then we need to toss it back onto the
                    // work list to be considered in the next round.
                    //
                    // TODO: We need to carefully vet that this can
                    // never lead to infinite looping
                    //
                    workList.Add(inst);
                }
                else
                {
                    // If all of the operands to `inst` are
                    // fully specialized, then we can go
                    // ahead and do whatever is required
                    // to "fully specialize" `inst` itself.
                    //
                    fullySpecializeInst(inst);

                    // At this point, we want to start
                    // considering `inst` as fully specialized,
                    // so let's add it to our set.
                    //
                    fullySpecializedInsts.Add(inst);
                }
            }
        }

        // Once the work list has gone dry, we should have the invariant
        // that there are no `specialize` instructions inside of non-generic
        // functions that in turn reference a generic type/function, *except*
        // in the case where that generic is for a builtin type/function, in
        // which case we wouldn't want to specialize it anyway.
    }
};

void specializeGenerics(
    IRModule*   module)
{
    SpecializationContext context;
    context.processModule(module);
}

} // namespace Slang
