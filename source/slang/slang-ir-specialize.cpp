// slang-ir-specialize.cpp
#include "slang-ir-specialize.h"

#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

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
// This pass also performs some amount of simplification and
// specialization for code using existential (interface) types
// for local variables and function parameters/results.
//
// Eventually, this pass will also need to perform specialization
// of functions to argument values for parameters that must
// be compile-time constants,
//
// All of these passes are inter-related in that applying
// simplifications/specializations of one category can open
// up opportunities for transformations in the other categories.

struct SpecializationContext;

IRInst* specializeGenericImpl(
    IRGeneric*              genericVal,
    IRSpecialize*           specializeInst,
    IRModule*               module,
    SpecializationContext*  context);

struct SpecializationContext
{
    // For convenience, we will keep a pointer to the module
    // we are specializing.
    IRModule* module;

    // We know that we can only perform generic specialization when all
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

    // We will use a single work list of instructions that need
    // to be considered for specialization or simplification,
    // whether generic, existential, etc.
    //
    List<IRInst*> workList;
    HashSet<IRInst*> workListSet;

    HashSet<IRInst*> cleanInsts;

    void addToWorkList(
        IRInst* inst)
    {
        // We will ignore any code that is nested under a generic,
        // because it doesn't make sense to perform specialization
        // on such code.
        //
        for( auto ii = inst->getParent(); ii; ii = ii->getParent() )
        {
            if(as<IRGeneric>(ii))
                return;
        }

        if(workListSet.Contains(inst))
            return;

        workList.add(inst);
        workListSet.Add(inst);
        cleanInsts.Remove(inst);

        addUsersToWorkList(inst);
    }

    // When a transformation makes a change to an instruction,
    // we may need to re-consider transformations for instructions
    // that use its value. In those cases we will call `addUsersToWorkList`
    // on the instruction that is being modified or replaced.
    //
    void addUsersToWorkList(
        IRInst* inst)
    {
        for( auto use = inst->firstUse; use; use = use->nextUse )
        {
            auto user = use->getUser();
            
            addToWorkList(user);
        }
    }

    // One of the main transformations we will apply is to
    // consider an instruction as being fully specialized.
    //
    void markInstAsFullySpecialized(
        IRInst* inst)
    {
        if(fullySpecializedInsts.Contains(inst))
            return;
        fullySpecializedInsts.Add(inst);

        // If we know that an instruction is fully specialized,
        // then we should start to consider its uses and children
        // as candidates for being fully specialized too...
        //
        addUsersToWorkList(inst);
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
        key.vals.add(specializeInst->getBase());
        UInt argCount = specializeInst->getArgCount();
        for( UInt ii = 0; ii < argCount; ++ii )
        {
            key.vals.add(specializeInst->getArg(ii));
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
        // This mostly amounts to evaluating the generic as
        // if it were a function being called.
        //
        // We will use a free function to do the actual work
        // of evaluating the generic, so that the logic
        // can be re-used in other cases that need to
        // do one-off specialization.
        //
        IRInst* specializedVal = specializeGenericImpl(genericVal, specializeInst, module, this);


        // The value that was returned from evaluating
        // the generic is the specialized value, and we
        // need to remember it in our dictionary of
        // specializations so that we don't instantiate
        // this generic again for the same arguments.
        //
        genericSpecializations.Add(key, specializedVal);

        return specializedVal;
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
            // We can't specialize a generic if it is marked as
            // being imported from an external module (in which
            // case its definition is not available to us).
            //
            if(!isDefinition(g))
                return false;

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

            // We should never specialize intrinsic types.
            //
            // TODO: This logic assumes that having *any* target
            // intrinsic decoration makes a type skip specialization,
            // even if the decoration isn't applicable to the
            // current target. This should be made true in practice
            // by having the linking step strip/skip decorations
            // that aren't applicable to the chosen target at link time.
            //
            if(as<IRStructType>(val) && val->findDecoration<IRTargetIntrinsicDecoration>())
                return false;

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
        // We will only attempt to specialize when all of the
        // operands to the `speicalize(...)` instruction are
        // themselves fully specialized.
        //
        if(!areAllOperandsFullySpecialized(specInst))
            return;

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

        // Any uses of this `specialize(...)` instruction will
        // become uses of `specializeVal`, so we want to re-consider
        // them for subsequent transformations.
        //
        addUsersToWorkList(specInst);

        // Then we simply replace any uses of the `specialize(...)`
        // instruction with the specialized value and delete
        // the `specialize(...)` instruction from existence.
        //
        specInst->replaceUsesWith(specializedVal);
        specInst->removeAndDeallocate();
    }

    // Generic specialization depends on identifying when
    // instructions are fully specialized.
    //
    void maybeMarkAsFullySpecialized(
        IRInst* inst)
    {
        // TODO: The logic here is completely bogus and
        // we need to revisit the notion of fully-specialized-ness
        // to only involve things that are semantically *values*
        // rather than computations/expressions.
        //
        // The rules should be something like:
        //
        // * Literals are values
        // * Composite type constructors where all the operands are value are values
        // * References to nominal types are values
        // * Built-in types where all the operands are values are values
        //
        // The system for defining value-ness probably needs
        // to combine with the system for deduplicating instructions,
        // since values are an important class of instruction we want
        // to deduplicate.

        switch(inst->op)
        {
        default:
            // The default case is that an instruction can
            // be considered as fully specialized as soon
            // as all of its operands are.
            //
            // TODO: We realistically need a more refined
            // check here that uses a white-list of instructions
            // that can represent values suitable for use
            // as generic arguments.
            //
            if(areAllOperandsFullySpecialized(inst))
            {
                markInstAsFullySpecialized(inst);
            }
            break;

            // Certain instructions cannot ever be considered
            // fully specialized because they should never
            // be substituted into a generic as its arguments.
        case kIROp_lookup_interface_method:
        case kIROp_ExtractExistentialType:
        case kIROp_BindExistentialsType:
            break;

            // An interface type is always fully specialized.
        case kIROp_InterfaceType:
            markInstAsFullySpecialized(inst);
            break;

        case kIROp_Specialize:
            // The `specialize` instruction is a bit sepcial,
            // because it is possible to have a `specialize`
            // of a built-in type so that it never gets
            // substituted for another type. (e.g., the specific
            // case where this code path first showed up
            // as necessary was `RayQuery<>`)
            //
            {
                auto specialize = cast<IRSpecialize>(inst);
                auto base = specialize->getBase();
                if( auto generic = as<IRGeneric>(base) )
                {
                    // If the thing being specialized can be resolved,
                    // *and* it is a target intrinsic, ...
                    //
                    if( auto result = findGenericReturnVal(generic) )
                    {
                        if( result->findDecoration<IRTargetIntrinsicDecoration>() )
                        {
                            // ... then we should consider the instruction as
                            // "fully specialized" in the same cases as for
                            // any ordinary instruciton.
                            //

                            if( areAllOperandsFullySpecialized(inst) )
                            {
                                markInstAsFullySpecialized(inst);
                            }
                            return;
                        }
                    }
                }

                // Otherwise, a `specialize` instruction falls into
                // the case of instructions that should never be
                // considered to be fully specialized.
            }
            break;
        }
    }

    // The core of this pass is to look at one instruction
    // at a time, and try to perform whatever specialization
    // is appropriate based on its opcode.
    //
    void maybeSpecializeInst(
        IRInst*                     inst)
    {
        switch(inst->op)
        {
        default:
            // By default we assume that specialization is
            // not possible for a given opcode.
            //
            break;

        case kIROp_Specialize:
            // The logic for specializing a `specialize(...)`
            // instruction has already been elaborated above.
            //
            maybeSpecializeGeneric(cast<IRSpecialize>(inst));
            break;

        case kIROp_lookup_interface_method:
            // The remaining case we need to consider here for generics
            // is when we have a `lookup_witness_method` instruction
            // that is being applied to a concrete witness table,
            // because we can specialize it to just be a direct
            // reference to the actual witness value from the table.
            //
            maybeSpecializeWitnessLookup(cast<IRLookupWitnessMethod>(inst));
            break;

        case kIROp_Call:
            // When writing functions with existential-type parameters,
            // we need additional support to specialize a callee
            // function based on the concrete type encapsulated in
            // an argument of existential type.
            //
            maybeSpecializeExistentialsForCall(cast<IRCall>(inst));
            break;

            // The specialization of functions with existential-type
            // parameters can create further opportunities for specialization,
            // but in order to realize these we often need to propagate
            // through local simplification on values of existential type.
            //
        case kIROp_ExtractExistentialType:
            maybeSpecializeExtractExistentialType(inst);
            break;
        case kIROp_ExtractExistentialValue:
            maybeSpecializeExtractExistentialValue(inst);
            break;
        case kIROp_ExtractExistentialWitnessTable:
            maybeSpecializeExtractExistentialWitnessTable(inst);
            break;

        case kIROp_Load:
            maybeSpecializeLoad(as<IRLoad>(inst));
            break;

        case kIROp_FieldExtract:
            maybeSpecializeFieldExtract(as<IRFieldExtract>(inst));
            break;
        case kIROp_FieldAddress:
            maybeSpecializeFieldAddress(as<IRFieldAddress>(inst));
            break;

        case kIROp_BindExistentialsType:
            maybeSpecializeBindExistentialsType(as<IRBindExistentialsType>(inst));
            break;
        }
    }

    // Specializing lookup on witness tables is a general
    // transformation that helps with both generic and
    // existential-based code. 
    //
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
        // instruction dynamically, so we can go ahead and
        // replace the original instruction with that value.
        //
        // We also make sure to add any uses of the lookup
        // instruction to our work list, because subsequent
        // simplifications might be possible now.
        //
        addUsersToWorkList(lookupInst);
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

    // All of the machinery for generic specialization
    // has been defined above, so we will now walk
    // through the flow of the overall specialization pass.
    //
    void processModule()
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
        // TODO: When we start to support global shader parameters
        // that include existential/interface types, we will need
        // to support a similar specialization step for them.
        //
        specializeGlobalGenericParameters();

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
        // The basic approach now is to look for opportunities to apply
        // our specialization rules (e.g., a `specialize` instruction
        // where all the type arguments are concrete types) and then
        // processing any additional opportunities created along the way.
        //
        // We start out simple by putting the root instruction for the
        // module onto our work list.
        //
        addToWorkList(module->getModuleInst());

        while(workList.getCount() != 0)
        {

        // We will then iterate until our work list goes dry.
        //
        while(workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.Remove(inst);
            cleanInsts.Add(inst);

            // For each instruction we process, we want to perform
            // a few steps.
            //
            // First we will do any checking required to tag an
            // instruction as being fully specialized.
            //
            maybeMarkAsFullySpecialized(inst);

            // Next we will look for all the general-purpose
            // specialization opportunities (generic specialization,
            // existential specialization, simplifications, etc.)
            //
            maybeSpecializeInst(inst);

            // Finally, we need to make our logic recurse through
            // the whole IR module, so we want to add the children
            // of any parent instructions to our work list so that
            // we process them too.
            //
            // Note that we are adding the children of an instruction
            // in reverse order. This is because the way we are
            // using the work list treats it like a stack (LIFO) and
            // we know that fully-specialized-ness will tend to flow
            // top-down through the program, so that we want to process
            // the children of an instruction in their original order.
            //
            for(auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                // Also note that `addToWorkList` has been written
                // to avoid adding any instruction that is a descendent
                // of an IR generic, because we don't actually want
                // to perform specialization inside of generics.
                //
                addToWorkList(child);
            }
        }

        addDirtyInstsToWorkListRec(module->getModuleInst());

        }

        // Once the work list has gone dry, we should have the invariant
        // that there are no `specialize` instructions inside of non-generic
        // functions that in turn reference a generic type/function, *except*
        // in the case where that generic is for a builtin type/function, in
        // which case we wouldn't want to specialize it anyway.
    }

    void addDirtyInstsToWorkListRec(IRInst* inst)
    {
        if( !cleanInsts.Contains(inst) )
        {
            addToWorkList(inst);
        }

        for(auto child = inst->getLastChild(); child; child = child->getPrevInst())
        {
            addDirtyInstsToWorkListRec(child);
        }
    }

    // Given a `call` instruction in the IR, we need to detect the case
    // where the callee has some interface-type parameter(s) and at the
    // call site it is statically clear what concrete type(s) the arguments
    // will have.
    //
    void maybeSpecializeExistentialsForCall(IRCall* inst)
    {
        // We can only specialize a call when the callee function is known.
        //
        auto calleeFunc = as<IRFunc>(inst->getCallee());
        if(!calleeFunc)
            return;

        // We can only specialize if we have access to a body for the callee.
        //
        if(!calleeFunc->isDefinition())
            return;

        // We shouldn't bother specializing unless the callee has at least
        // one parameter that has an existential/interface type.
        //
        bool shouldSpecialize = false;
        UInt argCounter = 0;
        for( auto param : calleeFunc->getParams() )
        {
            auto arg = inst->getArg(argCounter++);
            if( !isExistentialType(param->getDataType()) )
                continue;

            shouldSpecialize = true;

            // We *cannot* specialize unless the argument value corresponding
            // to such a parameter is one we can specialize.
            //
            if( !canSpecializeExistentialArg(arg))
                return;

        }
        // If we never found a parameter worth specializing, we should bail out.
        //
        if(!shouldSpecialize)
            return;

        // At this point, we believe we *should* and *can* specialize.
        //
        // We need a specialized variant of the callee (with the concrete
        // types substituted in for existential-type parameters), and then
        // we can replace the call site to call the new function instead.
        //
        // Any two call sites where the argument types are the same can
        // re-use the same callee, so we will  cache and re-use the
        // specialized functions that we generate (similar to how generic
        // specialization works). Therefore we will construct a key
        // for use when caching the specialized functions.
        // 
        IRSimpleSpecializationKey key;

        // The specialized callee will always depend on the unspecialized
        // function from which it is generated, so we add that to our key.
        //
        key.vals.add(calleeFunc);

        // Also, for any parameter that has an existential type, the
        // specialized function will depend on the concrete type of the
        // argument.
        //
        argCounter = 0;
        for( auto param : calleeFunc->getParams() )
        {
            auto arg = inst->getArg(argCounter++);
            if( !isExistentialType(param->getDataType()) )
                continue;

            if( auto makeExistential = as<IRMakeExistential>(arg) )
            {
                // Note that we use the *type* stored in the
                // existential-type argument, but not anything to
                // do with the particular value (otherwise we'd only
                // be able to re-use the specialized callee for
                // call sites that pass in the exact same argument).
                //
                auto val = makeExistential->getWrappedValue();
                auto valType = val->getFullType();
                key.vals.add(valType);

                // We are also including the witness table in the key.
                // This isn't required with our current language model,
                // since a given type can only conform to a given interface
                // in one way (so there can be only one witness table).
                // That means that the `valType` and the existential
                // type of `param` above should uniquely determine
                // the witness table we see.
                //
                // There are forward-looking cases where supporting
                // "overlapping conformances" could be required, and
                // there is low incremental cost to future-proofing
                // this code, so we go ahead and add the witness
                // table even if it is redundant.
                //
                auto witnessTable = makeExistential->getWitnessTable();
                key.vals.add(witnessTable);
            }
            else if( auto wrapExistential = as<IRWrapExistential>(arg) )
            {
                auto val = wrapExistential->getWrappedValue();
                auto valType = val->getFullType();
                key.vals.add(valType);

                UInt slotOperandCount = wrapExistential->getSlotOperandCount();
                for( UInt ii = 0; ii < slotOperandCount; ++ii )
                {
                    auto slotOperand = wrapExistential->getSlotOperand(ii);
                    key.vals.add(slotOperand);
                }
            }
            else
            {
                SLANG_UNEXPECTED("missing case for existential argument");
            }
        }

        // Once we've constructed our key, we can try to look for an
        // existing specialization of the callee that we can use.
        //
        IRFunc* specializedCallee = nullptr;
        if( !existentialSpecializedFuncs.TryGetValue(key, specializedCallee) )
        {
            // If we didn't find a specialized callee already made, then we
            // will go ahead and create one, and then register it in our cache.
            //
            specializedCallee = createExistentialSpecializedFunc(inst, calleeFunc);
            existentialSpecializedFuncs.Add(key, specializedCallee);
        }

        // At this point we have found or generated a specialized version
        // of the callee, and we need to emit a call to it.
        //
        // We will start by constructing the argument list for the new call.
        //
        argCounter = 0;
        List<IRInst*> newArgs;
        for( auto param : calleeFunc->getParams() )
        {
            auto arg = inst->getArg(argCounter++);

            // How we handle each argument depends on whether the corresponding
            // parameter has an existential type or not.
            //
            if( !isExistentialType(param->getDataType()) )
            {
                // If the parameter doesn't have an existential type, then we
                // don't want to change up the argument we pass at all.
                //
                newArgs.add(arg);
            }
            else
            {
                // Any place where the original function had a parameter of
                // existential type, we will now be passing in the concrete
                // argument value instead of an existential wrapper.
                //
                if( auto makeExistential = as<IRMakeExistential>(arg) )
                {
                    auto val = makeExistential->getWrappedValue();
                    newArgs.add(val);
                }
                else if( auto wrapExistential = as<IRWrapExistential>(arg) )
                {
                    auto val = wrapExistential->getWrappedValue();
                    newArgs.add(val);
                }
                else
                {
                    SLANG_UNEXPECTED("missing case for existential argument");
                }
            }
        }

        // Now that we've built up our argument list, it is simple enough
        // to construct a new `call` instruction.
        //
        IRBuilder builderStorage;
        auto builder = &builderStorage;
        builder->sharedBuilder = &sharedBuilderStorage;

        builder->setInsertBefore(inst);
        auto newCall = builder->emitCallInst(
            inst->getFullType(), specializedCallee, newArgs);

        // We will completely replace the old `call` instruction with the
        // new one, and will go so far as to transfer any decorations
        // that were attached to the old call over to the new one.
        //
        inst->transferDecorationsTo(newCall);
        inst->replaceUsesWith(newCall);
        inst->removeAndDeallocate();

        // Just in case, we will add any instructions that used the
        // result of this call to our work list for re-consideration.
        // At this moment this shouldn't open up new opportunities
        // for specialization, but we can always play it safe.
        //
        addUsersToWorkList(newCall);
    }

    // The above `maybeSpecializeExistentialsForCall` routine needed
    // a few utilities, which we will now define.

    // First, we want to be able to test whether a type (used by
    // a parameter) is an existential type so that we should specialize it.
    //
    bool isExistentialType(IRType* type)
    {
        // An IR-level interface type is always an existential.
        //
        if(as<IRInterfaceType>(type))
            return true;

        // Eventually we will also want to handle arrays over
        // existential types, but that will require careful
        // handling in many places.

        return false;
    }

    // Similarly, we want to be able to test whether an instruction
    // used as an argument for an existential-type parameter is
    // suitable for use in specialization.
    //
    bool canSpecializeExistentialArg(IRInst* inst)
    {
        // A `makeExistential(v, w)` instruction can be used
        // for specialization, since we have the concrete value `v`
        // (which implicitly determines the concrete type), and
        // the witness table `w.
        //
        if(as<IRMakeExistential>(inst))
            return true;

        // A `wrapExistential(v, T0,w0, T1, w1, ...)` instruction
        // is just a generalization of `makeExistential`, so it
        // can apply in the same cases.
        //
        if(as<IRWrapExistential>(inst))
            return true;

        // If we start to specialize functions that take arrays
        // of existentials as input, we will need a strategy to
        // determine arguments suitable for use in specializing
        // them (these would need to be arrays that nominally
        // have an existential element type, but somehow have
        // annotations to indicate that the concrete type
        // underlying the elements in homogeneous).

        return false;
    }

    // In order to cache and re-use functions that have had existential-type
    // parameters specialized, we need storage for the cache.
    //
    Dictionary<IRSimpleSpecializationKey, IRFunc*> existentialSpecializedFuncs;

    // The logic for creating a specialized callee function by plugging
    // in concrete types for existentials is similar to other cases of
    // specialization in the compiler.
    //
    IRFunc* createExistentialSpecializedFunc(
        IRCall* oldCall,
        IRFunc* oldFunc)
    {
        // We will make use of the infrastructure for cloning
        // IR code, that is defined in `ir-clone.{h,cpp}`.
        //
        // In order to do the cloning work we need an
        // "environment" that will map old values to
        // their replacements.
        //
        IRCloneEnv cloneEnv;

        // We also need some IR building state, for any
        // new instructions we will emit.
        //
        IRBuilder builderStorage;
        auto builder = &builderStorage;
        builder->sharedBuilder = &sharedBuilderStorage;

        // We will start out by determining what the parameters
        // of the specialized function should be, based on
        // the parameters of the original, and the concrete
        // type of selected arguments at the call site.
        //
        // Along the way we will build up explicit lists of
        // the parameters, as well as any new instructions
        // that need to be added to the body of the function
        // we generate (as a kind of "prologue"). We build
        // the lists here because we don't yet have a basic
        // block, or even a function, to insert them into.
        //
        List<IRParam*> newParams;
        List<IRInst*> newBodyInsts;
        UInt argCounter = 0;
        for( auto oldParam : oldFunc->getParams() )
        {
            auto arg = oldCall->getArg(argCounter++);

            // Given an old parameter, and the argument value at
            // the (old) call site, we need to determine what
            // value should stand in for that parameter in
            // the specialized callee.
            //
            IRInst* replacementVal = nullptr;

            // The trickier case is when we have an existential-type
            // parameter, because we need to extract out the concrete
            // type that is coming from the call site.
            //
            if( auto oldMakeExistential = as<IRMakeExistential>(arg) )
            {
                // In this case, the `arg` is `makeExistential(val, witnessTable)`
                // and we know that the specialized call site will just be
                // passing in `val`.
                //
                auto val = oldMakeExistential->getWrappedValue();
                auto witnessTable = oldMakeExistential->getWitnessTable();

                // Our specialized function needs to take a parameter with the
                // same type as `val`, to match the call site(s) that will be
                // created.
                //
                auto valType = val->getFullType();
                auto newParam = builder->createParam(valType);
                newParams.add(newParam);

                // Within the body of the function we cannot just use `val`
                // directly, because the existing code expects an existential
                // value, including its witness table.
                //
                // Therefore we will create a `makeExistential(newParam, witnessTable)`
                // in the body of the new function and use *that* as the replacement
                // value for the original parameter (since it will have the
                // correct existential type, and stores the right witness table).
                //
                auto newMakeExistential = builder->emitMakeExistential(oldParam->getFullType(), newParam, witnessTable);
                newBodyInsts.add(newMakeExistential);
                replacementVal = newMakeExistential;
            }
            else if( auto oldWrapExistential = as<IRWrapExistential>(arg) )
            {
                auto val = oldWrapExistential->getWrappedValue();
                auto valType = val->getFullType();

                auto newParam = builder->createParam(valType);
                newParams.add(newParam);

                // Within the body of the function we cannot just use `val`
                // directly, because the existing code expects an existential
                // value, including its witness table.
                //
                // Therefore we will create a `makeExistential(newParam, witnessTable)`
                // in the body of the new function and use *that* as the replacement
                // value for the original parameter (since it will have the
                // correct existential type, and stores the right witness table).
                //
                auto newWrapExistential = builder->emitWrapExistential(
                    oldParam->getFullType(),
                    newParam,
                    oldWrapExistential->getSlotOperandCount(),
                    oldWrapExistential->getSlotOperands());
                newBodyInsts.add(newWrapExistential);
                replacementVal = newWrapExistential;
            }
            else
            {
                // For parameters that don't have an existential type,
                // there is nothing interesting to do. The new function
                // will also have a parameter of the exact same type,
                // and we'll use that instead of the original parameter.
                //
                auto newParam = builder->createParam(oldParam->getFullType());
                newParams.add(newParam);
                replacementVal = newParam;
            }

            // Whatever replacement value was constructed, we need to
            // register it as the replacement for the original parameter.
            //
            cloneEnv.mapOldValToNew.Add(oldParam, replacementVal);
        }

        // Next we will create the skeleton of the new
        // specialized function, including its type.
        //
        // In order to construct the type of the new function, we
        // need to extract the types of all its parameters.
        //
        List<IRType*> newParamTypes;
        for( auto newParam : newParams )
        {
            newParamTypes.add(newParam->getFullType());
        }
        IRType* newFuncType = builder->getFuncType(
            newParamTypes.getCount(),
            newParamTypes.getBuffer(),
            oldFunc->getResultType());
        IRFunc* newFunc = builder->createFunc();
        newFunc->setFullType(newFuncType);

        // By construction, our new function type will be
        // "fully specialized" by the rules used for doing
        // generic specialization elsewhere in this pass.
        //
        fullySpecializedInsts.Add(newFuncType);

        // The above steps have accomplished the "first phase"
        // of cloning the function (since `IRFunc`s have no
        // operands).
        //
        // We can now use the shared IR cloning infrastructure
        // to perform the second phase of cloning, which will recursively
        // clone any nested decorations, blocks, and instructions.
        //
        cloneInstDecorationsAndChildren(
            &cloneEnv,
            builder->sharedBuilder,
            oldFunc,
            newFunc);

        // Now that the main body of existing isntructions have
        // been cloned into the new function, we can go ahead
        // and insert all the parameters and body instructions
        // we built up into the function at the right place.
        //
        // We expect the function to always have at least one
        // block (this was an invariant established before
        // we decided to specialize).
        //
        auto newEntryBlock = newFunc->getFirstBlock();
        SLANG_ASSERT(newEntryBlock);

        // We expect every valid block to have at least one
        // "ordinary" instruction (it will at least have
        // a terminator like a `return`).
        //
        auto newFirstOrdinary = newEntryBlock->getFirstOrdinaryInst();
        SLANG_ASSERT(newFirstOrdinary);

        // All of our parameters will get inserted before
        // the first ordinary instruction (since the function parameters
        // should come at the start of the first block).
        //
        for( auto newParam : newParams )
        {
            newParam->insertBefore(newFirstOrdinary);
        }

        // All of our new body instructions will *also* be inserted
        // before the first ordinary instruction (but will come
        // *after* the parameters by the order of these two loops).
        //
        for( auto newBodyInst : newBodyInsts )
        {
            newBodyInst->insertBefore(newFirstOrdinary);
        }

        // After all this work we have a valid `newFunc` that has been
        // specialized to match the types at the call site.
        //
        // There might be further opportunities for simplification and
        // specialization in the function body now that we've plugged
        // in some more concrete type information, so we will
        // add the whole function to our work list for subsequent
        // consideration.
        //
        addToWorkList(newFunc);

        return newFunc;
    }

    // When we've specialized a function with an interface-type parameter
    // we will still end up with a `makeExistential` operation in its
    // body, which could impede subequent specializations.
    //
    // For example, if we have the following after specialization:
    //
    //      e = makeExistential(v, w1);
    //      w2 = extractExistentialWitnessTable(e);
    //      f = lookup_witness_method(w2, k);
    //      call(f, ...);
    //
    // We cannot then specialize the lookup for `f` in this code as written,
    // but it seems obvious that we could replace `w2` with `w1` and maybe
    // get further along.
    //
    // In order to set up further specialization opportunities we need
    // to implement a few simplification rules around operations that
    // extract from an existential, when their operand is a `makeExistential`.
    //
    // Let's start with the routine for the case above of extracting
    // a witness table.
    //
    void maybeSpecializeExtractExistentialWitnessTable(IRInst* inst)
    {
        // We know `inst` is `extractExistentialWitnessTable(existentialArg)`.
        //
        auto existentialArg = inst->getOperand(0);

        if( auto makeExistential = as<IRMakeExistential>(existentialArg) )
        {
            // In this case we know `inst` is:
            //
            //      extractExistentialWitnessTable(makeExistential(..., witnessTable))
            //
            // and we can just simplify that to `witnessTable`.
            //
            auto witnessTable = makeExistential->getWitnessTable();

            // Anything that used this instruction is now a candidate for
            // further simplification or specialization (e.g., one of
            // the users of this instruction could be a `lookup_witness_method`
            // that we can now specialize).
            //
            addUsersToWorkList(inst);

            inst->replaceUsesWith(witnessTable);
            inst->removeAndDeallocate();
        }
    }

    // The cases for simplifying `extractExistentialValue` is more or less the same
    // as for witness tables.
    //
    void maybeSpecializeExtractExistentialValue(IRInst* inst)
    {
        // We know `inst` is `extractExistentialValue(existentialArg)`.
        //
        auto existentialArg = inst->getOperand(0);
        if( auto makeExistential = as<IRMakeExistential>(existentialArg) )
        {
            // Now we know `inst` is:
            //
            //      extractExistentialValue(makeExistential(val, ...))
            //
            // and we can just simplify that to `val`.
            //
            auto val = makeExistential->getWrappedValue();

            addUsersToWorkList(inst);

            inst->replaceUsesWith(val);
            inst->removeAndDeallocate();
        }
    }

    // The cases for simplifying `extractExistentialType` is more or less the same
    // as for witness tables.
    //
    void maybeSpecializeExtractExistentialType(IRInst* inst)
    {
        // We know `inst` is `extractExistentialValue(existentialArg)`.
        //
        auto existentialArg = inst->getOperand(0);
        if( auto makeExistential = as<IRMakeExistential>(existentialArg) )
        {
            // Now we know `inst` is:
            //
            //      extractExistentialType(makeExistential(val, ...))
            //
            // and we can just simplify that to type type of `val`.
            //
            auto val = makeExistential->getWrappedValue();
            auto valType = val->getFullType();

            addUsersToWorkList(inst);

            inst->replaceUsesWith(valType);
            inst->removeAndDeallocate();
        }
    }

    void maybeSpecializeLoad(IRLoad* inst)
    {
        auto ptrArg = inst->ptr.get();

        if( auto wrapInst = as<IRWrapExistential>(ptrArg) )
        {
            // We have an instruction of the form `load(wrapExistential(val, ...))`
            //
            auto val = wrapInst->getWrappedValue();

            // We know what type we are expected to
            // produce (which should be the pointed-to
            // type for whatever the type of the
            // `wrapExistential` is).
            //
            auto resultType = inst->getFullType();

            IRBuilder builder;
            builder.sharedBuilder = &sharedBuilderStorage;
            builder.setInsertBefore(inst);

            // We'd *like* to replace this instruction with
            // `wrapExistential(load(val))` instead, since that
            // will enable subsequent specializations.
            //
            // To do that, we need to be able to determine
            // the type that `load(val)` should return.
            //
            auto elementType = tryGetPointedToType(&builder, val->getDataType());
            if(!elementType)
                return;


            List<IRInst*> slotOperands;
            UInt slotOperandCount = wrapInst->getSlotOperandCount();
            for( UInt ii = 0; ii < slotOperandCount; ++ii )
            {
                slotOperands.add(wrapInst->getSlotOperand(ii));
            }

            auto newLoadInst = builder.emitLoad(elementType, val);
            auto newWrapExistentialInst = builder.emitWrapExistential(
                resultType,
                newLoadInst,
                slotOperandCount,
                slotOperands.getBuffer());

            addUsersToWorkList(inst);

            inst->replaceUsesWith(newWrapExistentialInst);
            inst->removeAndDeallocate();
        }
    }

    UInt calcExistentialBoxSlotCount(IRType* type)
    {
    top:
        if( as<IRExistentialBoxType>(type) )
        {
            return 2;
        }
        else if( auto ptrType = as<IRPtrTypeBase>(type) )
        {
            type = ptrType->getValueType();
            goto top;
        }
        else if( auto ptrLikeType = as<IRPointerLikeType>(type) )
        {
            type = ptrLikeType->getElementType();
            goto top;
        }
        else if( auto structType = as<IRStructType>(type) )
        {
            UInt count = 0;
            for( auto field : structType->getFields() )
            {
                count += calcExistentialBoxSlotCount(field->getFieldType());
            }
            return count;
        }
        else
        {
            return 0;
        }
    }

    void maybeSpecializeFieldExtract(IRFieldExtract* inst)
    {
        auto baseArg = inst->getBase();
        auto fieldKey = inst->getField();

        if( auto wrapInst = as<IRWrapExistential>(baseArg) )
        {
            // We have `getField(wrapExistential(val, ...), fieldKey)`
            //
            auto val = wrapInst->getWrappedValue();

            // We know what type we are expected to produce.
            //
            auto resultType = inst->getFullType();

            IRBuilder builder;
            builder.sharedBuilder = &sharedBuilderStorage;
            builder.setInsertBefore(inst);

            // We'd *like* to replace this instruction with
            // `wrapExistential(getField(val, fieldKey), ...)` instead, since that
            // will enable subsequent specializations.
            //
            // To do that, we need to figure out:
            //
            // 1. What type that inner `getField` would return (what
            // is the type of the `fieldKey` field in `val`?)
            //
            // 2. Which of the existential slot operands in `...` there
            // actually apply to the given field.
            //

            // To determine these things, we need the type of
            // `val` to be a structure type so that we can look
            // up the field corresponding to `fieldKey`.
            //
            auto valType = val->getDataType();
            auto valStructType = as<IRStructType>(valType);
            if(!valStructType)
                return;

            UInt slotOperandOffset = 0;

            IRStructField* foundField = nullptr;
            for( auto valField : valStructType->getFields() )
            {
                if( valField->getKey() == fieldKey )
                {
                    foundField = valField;
                    break;
                }

                slotOperandOffset += calcExistentialBoxSlotCount(valField->getFieldType());
            }

            if(!foundField)
                return;

            auto foundFieldType = foundField->getFieldType();

            List<IRInst*> slotOperands;
            UInt slotOperandCount = calcExistentialBoxSlotCount(foundFieldType);

            for( UInt ii = 0; ii < slotOperandCount; ++ii )
            {
                slotOperands.add(wrapInst->getSlotOperand(slotOperandOffset + ii));
            }

            auto newGetField = builder.emitFieldExtract(
                foundFieldType,
                val,
                fieldKey);

            auto newWrapExistentialInst = builder.emitWrapExistential(
                resultType,
                newGetField,
                slotOperandCount,
                slotOperands.getBuffer());

            addUsersToWorkList(inst);
            inst->replaceUsesWith(newWrapExistentialInst);
            inst->removeAndDeallocate();
        }
    }


    void maybeSpecializeFieldAddress(IRFieldAddress* inst)
    {
        auto baseArg = inst->getBase();
        auto fieldKey = inst->getField();

        if( auto wrapInst = as<IRWrapExistential>(baseArg) )
        {
            // We have `getFieldAddr(wrapExistential(val, ...), fieldKey)`
            //
            auto val = wrapInst->getWrappedValue();

            // We know what type we are expected to produce.
            //
            auto resultType = inst->getFullType();

            IRBuilder builder;
            builder.sharedBuilder = &sharedBuilderStorage;
            builder.setInsertBefore(inst);

            // We'd *like* to replace this instruction with
            // `wrapExistential(getFieldAddr(val, fieldKey), ...)` instead, since that
            // will enable subsequent specializations.
            //
            // To do that, we need to figure out:
            //
            // 1. What type that inner `getFieldAddr` would return (what
            // is the type of the `fieldKey` field in `val`?)
            //
            // 2. Which of the existential slot operands in `...` there
            // actually apply to the given field.
            //

            // To determine these things, we need the type of
            // `val` to be a (pointer to a) structure type so that we can look
            // up the field corresponding to `fieldKey`.
            //
            auto valType = tryGetPointedToType(&builder, val->getDataType());
            if(!valType)
                return;

            auto valStructType = as<IRStructType>(valType);
            if(!valStructType)
                return;

            UInt slotOperandOffset = 0;

            IRStructField* foundField = nullptr;
            for( auto valField : valStructType->getFields() )
            {
                if( valField->getKey() == fieldKey )
                {
                    foundField = valField;
                    break;
                }

                slotOperandOffset += calcExistentialBoxSlotCount(valField->getFieldType());
            }

            if(!foundField)
                return;

            auto foundFieldType = foundField->getFieldType();

            List<IRInst*> slotOperands;
            UInt slotOperandCount = calcExistentialBoxSlotCount(foundFieldType);

            for( UInt ii = 0; ii < slotOperandCount; ++ii )
            {
                slotOperands.add(wrapInst->getSlotOperand(slotOperandOffset + ii));
            }

            auto newGetFieldAddr = builder.emitFieldAddress(
                builder.getPtrType(foundFieldType),
                val,
                fieldKey);

            auto newWrapExistentialInst = builder.emitWrapExistential(
                resultType,
                newGetFieldAddr,
                slotOperandCount,
                slotOperands.getBuffer());

            addUsersToWorkList(inst);
            inst->replaceUsesWith(newWrapExistentialInst);
            inst->removeAndDeallocate();
        }
    }

    UInt calcExistentialTypeParamSlotCount(IRType* type)
    {
    top:
        if( as<IRInterfaceType>(type) )
        {
            return 2;
        }
        else if( auto ptrType = as<IRPtrTypeBase>(type) )
        {
            type = ptrType->getValueType();
            goto top;
        }
        else if( auto ptrLikeType = as<IRPointerLikeType>(type) )
        {
            type = ptrLikeType->getElementType();
            goto top;
        }
        else if( auto structType = as<IRStructType>(type) )
        {
            UInt count = 0;
            for( auto field : structType->getFields() )
            {
                count += calcExistentialTypeParamSlotCount(field->getFieldType());
            }
            return count;
        }
        else
        {
            return 0;
        }
    }

    Dictionary<IRSimpleSpecializationKey, IRStructType*> existentialSpecializedStructs;

    void maybeSpecializeBindExistentialsType(IRBindExistentialsType* type)
    {
        auto baseType = type->getBaseType();
        UInt slotOperandCount = type->getExistentialArgCount();

        IRBuilder builder;
        builder.sharedBuilder = &sharedBuilderStorage;
        builder.setInsertBefore(type);

        if( auto baseInterfaceType = as<IRInterfaceType>(baseType) )
        {
            // A `BindExistentials<ISomeInterface, ConcreteType, ...>` can
            // just be simplified to `ExistentialBox<ConcreteType>`.
            //
            // Note: We do *not* simplify straight to `ConcreteType`, because
            // that would mess up the layout for aggregate types that
            // contain interfaces. The logical indirection introduced
            // by `ExistentialBox<...>` will be handled by a later type
            // legalization pass that moved the type "pointed to" by
            // the box out of line from other fields.

            // We always expect two slot operands, one for the concrete type
            // and one for the witness table.
            //
            SLANG_ASSERT(slotOperandCount == 2);
            if(slotOperandCount <= 1) return;

            auto concreteType = (IRType*) type->getExistentialArg(0);
            auto newVal = builder.getPtrType(kIROp_ExistentialBoxType, concreteType);

            addUsersToWorkList(type);
            type->replaceUsesWith(newVal);
            type->removeAndDeallocate();
            return;
        }
        else if( auto basePtrLikeType = as<IRPointerLikeType>(baseType) )
        {
            // A `BindExistentials<P<T>, ...>` can be simplified to
            // `P<BindExistentials<T, ...>>` when `P` is a pointer-like
            // type constructor.
            //
            auto baseElementType = basePtrLikeType->getElementType();
            IRInst* wrappedElementType = builder.getBindExistentialsType(
                baseElementType,
                slotOperandCount,
                type->getExistentialArgs());
            addToWorkList(wrappedElementType);

            auto newPtrLikeType = builder.getType(
                basePtrLikeType->op,
                1,
                &wrappedElementType);
            addToWorkList(newPtrLikeType);

            addUsersToWorkList(type);
            type->replaceUsesWith(newPtrLikeType);
            type->removeAndDeallocate();
            return;
        }
        else if( auto baseStructType = as<IRStructType>(baseType) )
        {
            // In order to bind a `struct` type we will generate
            // a new specialized `struct` type on demand and then
            // cache and re-use it.
            //
            // We don't want to start specializing here unless
            // all the operand types (and witness tables) we
            // will be specializing to are themselves fully
            // specialized, so that we can be sure that we
            // have a unique type.
            //
            if( !areAllOperandsFullySpecialized(type) )
                return;

            // Now we we check to see if we've already created
            // a specialized struct type or not.
            //
            IRSimpleSpecializationKey key;
            key.vals.add(baseStructType);
            for( UInt ii = 0; ii < slotOperandCount; ++ii )
            {
                key.vals.add(type->getExistentialArg(ii));
            }

            IRStructType* newStructType = nullptr;
            if( !existentialSpecializedStructs.TryGetValue(key, newStructType) )
            {
                builder.setInsertBefore(baseStructType);
                newStructType = builder.createStructType();

                auto fieldSlotArgs = type->getExistentialArgs();

                for( auto oldField : baseStructType->getFields() )
                {
                    // TODO: we need to figure out which of the specialization arguments
                    // apply to this field...

                    auto oldFieldType = oldField->getFieldType();
                    auto fieldSlotArgCount = calcExistentialTypeParamSlotCount(oldFieldType);

                    auto newFieldType = builder.getBindExistentialsType(
                        oldFieldType,
                        fieldSlotArgCount,
                        fieldSlotArgs);

                    addToWorkList(newFieldType);

                    fieldSlotArgs += fieldSlotArgCount;

                    builder.createStructField(newStructType, oldField->getKey(), newFieldType);
                }

                existentialSpecializedStructs.Add(key, newStructType);
                addToWorkList(newStructType);
            }

            addUsersToWorkList(type);
            type->replaceUsesWith(newStructType);
            type->removeAndDeallocate();
            return;

        }
    }

    // The handling of specialization for global generic type
    // parameters involves searching for all `bind_global_generic_param`
    // instructions in the input module.
    //
    void specializeGlobalGenericParameters()
    {
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
    }
};

void specializeModule(
    IRModule*   module)
{
    SpecializationContext context;
    context.module = module;
    context.processModule();
}


IRInst* specializeGenericImpl(
    IRGeneric*              genericVal,
    IRSpecialize*           specializeInst,
    IRModule*               module,
    SpecializationContext*  context)
{
    // Effectively, specializing a generic amounts to "calling" the generic
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
    SharedIRBuilder sharedBuilderStorage;
    sharedBuilderStorage.module = module;
    sharedBuilderStorage.session = module->getSession();

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
            if( context )
            {
                context->addToWorkList(clonedInst);
            }
        }
    }

    // If we reach this point, something went wrong, because we
    // never encountered a `return` inside the body of the generic.
    //
    SLANG_UNEXPECTED("no return from generic");
    UNREACHABLE_RETURN(nullptr);
}

IRInst* specializeGeneric(
    IRSpecialize*   specializeInst)
{
    auto baseGeneric = as<IRGeneric>(specializeInst->getBase());
    SLANG_ASSERT(baseGeneric);
    if(!baseGeneric) return specializeInst;

    auto module = specializeInst->getModule();
    SLANG_ASSERT(module);
    if(!module) return specializeInst;

    return specializeGenericImpl(baseGeneric, specializeInst, module, nullptr);
}


} // namespace Slang
