// slang-ir-sccp.cpp
#include "slang-ir-sccp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang {


// This file implements the Spare Conditional Constant Propagation (SCCP) optimization.
//
// We will apply the optimization over individual functions, so we will start with
// a context struct for the state that we will share across functions:
//
struct SharedSCCPContext
{
    IRModule*       module;
    SharedIRBuilder sharedBuilder;
};
//
// Next we have a context struct that will be applied for each function (or other
// code-bearing value) that we optimize:
//
struct SCCPContext
{
    SharedSCCPContext*      shared; // shared state across functions
    IRGlobalValueWithCode*  code;   // the function/code we are optimizing

    // The SCCP algorithm applies abstract interpretation to the code of the
    // function using a "lattice" of values. We can think of a node on the
    // lattice as representing a set of values that a given instruction
    // might take on.
    //
    struct LatticeVal
    {
        // We will use three "flavors" of values on our lattice.
        //
        enum class Flavor
        {
            // The `None` flavor represent an empty set of values, meaning
            // that we've never seen any indication that the instruction
            // produces a (well-defined) value. This could indicate an
            // instruction that does not appear to execute, but it could
            // also indicate an instruction that we know invokes undefined
            // behavior, so we can freely pick a value for it on a whim.
            None,

            // The `Constant` flavor represents an instuction that we
            // have only ever seen produce a single, fixed value. It's
            // `value` field will hold that constant value.
            Constant,

            // The `Any` flavor represents an instruction that might produce
            // different values at runtime, so we go ahead and approximate
            // this as it potentially yielding any value whatsoever. A
            // more precise analysis could use sets or intervals of values,
            // but for SCCP anything that could take on more than 1 value
            // at runtime is assumed to be able to take on *any* value.
            Any,
        };

        // The flavor of this value (`None`, `Constant`, or `Any`)
        Flavor  flavor;

        // If this is a `Constant` lattice value, then this field
        // points to the IR instruction that defines the actual constant value.
        // For all other flavors it should be null.
        IRInst* value = nullptr;

        // For convenience, we define `static` factory functions to
        // produce values of each of the flavors.

        static LatticeVal getNone()
        {
            LatticeVal result;
            result.flavor = Flavor::None;
            return result;
        }

        static LatticeVal getAny()
        {
            LatticeVal result;
            result.flavor = Flavor::Any;
            return result;
        }

        static LatticeVal getConstant(IRInst* value)
        {
            LatticeVal result;
            result.flavor = Flavor::Constant;
            result.value = value;
            return result;
        }

        // We also need to be able to test if two lattice
        // values are equal, so that we can avoid updating
        // downstream dependencies if our knowledge about
        // an instruction hasn't actually changed.
        //
        bool operator==(LatticeVal const& that)
        {
            return this->flavor == that.flavor
                && this->value == that.value;
        }

        bool operator!=(LatticeVal const& that)
        {
            return !( *this == that );
        }
    };

    // If we imagine a variable (actually an SSA phi node...) that
    // might be assigned lattice value A at one point in the code,
    // and lattice value B at another point, we need a way to
    // combine these to form our knowledge of the possible value(s)
    // for the variable.
    //
    // In terms of computation on a lattice, we want the "meet"
    // operation, which computes the lower bound on what we know.
    // If we interpret our lattice values as sets, then we are
    // trying to compute the union.
    //
    LatticeVal meet(LatticeVal const& left, LatticeVal const& right)
    {
        // If either value is `None` (the empty set), then the union
        // will be the other value.
        //
        if(left.flavor == LatticeVal::Flavor::None) return right;
        if(right.flavor == LatticeVal::Flavor::None) return left;

        // If either value is `Any` (the universal set), then
        // the union is also the universal set.
        //
        if(left.flavor == LatticeVal::Flavor::Any) return LatticeVal::getAny();
        if(right.flavor == LatticeVal::Flavor::Any) return LatticeVal::getAny();

        // At this point we've ruled out the case where either value
        // is `None` *or* `Any`, so we can assume both values are
        // `Constant`s.
        SLANG_ASSERT(left.flavor == LatticeVal::Flavor::Constant);
        //
        SLANG_ASSERT(right.flavor == LatticeVal::Flavor::Constant);

        // If the two lattice values represent the *same* constant value
        // (they are the same singleton set) then the union is that
        // singleton set as well.
        //
        // TODO: This comparison assumes that constants with
        // the same value with be represented with the
        // same instruction, which is not *always*
        // guaranteed in the IR today.
        //
        if(left.value == right.value)
            return left;

        // Otherwise, we have two distinct singleton sets, and their
        // union should be a set with two elements. We can't represent
        // that on the lattice for SCCP, so the proper lower bound
        // is the universal set (`Any`)
        //
        return LatticeVal::getAny();
    }

    // During the execution of the SCCP algorithm, we will track our best
    // "estimate" so far of the set of values each instruction could take
    // on. This amounts to a mapping from IR instructions to lattice values,
    // where any instruction not present in the map is assumed to default
    // to the `None` case (the empty set)
    //
    Dictionary<IRInst*, LatticeVal> mapInstToLatticeVal;

    // Updating the lattice value for an instruction is easy, but we'll
    // use a simple function to make our intention clear.
    //
    void setLatticeVal(IRInst* inst, LatticeVal const& val)
    {
        mapInstToLatticeVal[inst] = val;
    }

    // Querying the lattice value for an instruction isn't *just* a matter
    // of looking it up in the dictionary, because we need to account for
    // cases of lattice values that might come from outside the current
    // function.
    //
    LatticeVal getLatticeVal(IRInst* inst)
    {
        // Instructions that represent constant values should always
        // have a lattice value that reflects this.
        //
        switch( inst->op )
        {
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_BoolLit:
            return LatticeVal::getConstant(inst);
            break;

        // TODO: We might want to start having support for constant
        // values of aggregate types (e.g., a `makeArray` or `makeStruct`
        // where all the operands are constant is itself a constant).

        default:
            break;
        }

        // We might be asked for the lattice value of an instruction
        // not contained in the current function. When that happens,
        // we will treat it as having potentially any value, rather
        // than the default of none.
        //
        auto parentBlock = as<IRBlock>(inst->getParent());
        if(!parentBlock || parentBlock->getParent() != code) return LatticeVal::getAny();

        // Once the special cases are dealt with, we can look up in
        // the dictionary and just return the value we get from it,
        // or default to the `None` (empty set) case.
        LatticeVal latticeVal;
        if(mapInstToLatticeVal.TryGetValue(inst, latticeVal))
            return latticeVal;
        return LatticeVal::getNone();
    }

    // Along the way we might need to create new IR instructions
    // to represnet new constant values we find, or new control
    // flow instructiosn when we start simplifying things.
    //
    IRBuilder builderStorage;
    IRBuilder* getBuilder() { return &builderStorage; }

    // In order to perform constant folding, we need to be able to
    // interpret an instruction over the lattice values.
    //
    LatticeVal interpretOverLattice(IRInst* inst)
    {
        SLANG_UNUSED(inst);

        // Certain instruction always produce constants, and we
        // want to special-case them here.
        switch( inst->op )
        {
        case kIROp_IntLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_BoolLit:
            return LatticeVal::getConstant(inst);

        // TODO: we might also want to special-case certain
        // instructions where we shouldn't bother trying to
        // constant-fold them and should just default to the
        // `Any` value right away.

        default:
            break;
        }

        // TODO: We should now look up the lattice values for
        // the operands of the instruction.
        //
        // If all of the operands have `Constant` lattice values,
        // then we can potential execute the operation directly
        // on those constant values, create a fresh `IRConstant`,
        // and return a `Constant` lattice value for it. This
        // would allow us to achieve true constant folding here.
        //
        // Textbook discussions of SCCP often point out that it
        // is also possible to perform certain algebraic simplifications
        // here, such as evaluating a multiply by a `Constant` zero
        // to zero.
        //
        // As a default, if any operand has the `Any` value
        // then the result of the operation should be treated as
        // `Any`. There are exceptions to this, however, with the
        // multiply-by-zero example being an important example.
        // If we had previously decided that (Any * None) -> Any
        // but then we refine our estimates and have (Any * Constant(0)) -> Constant(0)
        // then we have violated the monotonicity rules for how
        // our values move through the lattice, and we may break
        // the convergence guarantees of the analysis.
        //
        // When we have a mix of `None` and `Constant` operands,
        // then the `None` values imply that our operation is using
        // uninitialized data or the results of undefined behavior.
        // We could try to propagate the `None` through, and allow
        // the compiler to speculatively assume that the operation
        // produces whatever value we find convenient. Alternatively,
        // we can be less aggressive and treat an operation with
        // `None` inputs as producing `Any` to make sure we don't
        // optimize the code based on non-obvious assumptions.
        //
        // For now we aren't implementing *any* folding logic here,
        // for simplicity. This is the right place to add folding
        // optimizations if/when we need them.
        //

        // A safe default is to assume that every instruction not
        // handled by one of the cases above could produce *any*
        // value whatsoever.
        return LatticeVal::getAny();
    }


    // For basic blocks, we will do tracking very similar to what we do for
    // ordinary instructions, just with a simpler lattice: every block
    // will either be marked as "never executed" or in a "possibly executed"
    // state. We track this as a set of the blocks that have been
    // marked as possibly executed, plus a getter and setter function.

    HashSet<IRBlock*> executedBlocks;

    bool isMarkedAsExecuted(IRBlock* block)
    {
        return executedBlocks.Contains(block);
    }

    void markAsExecuted(IRBlock* block)
    {
        executedBlocks.Add(block);
    }

    // The core of the algorithm is based on two work lists.
    // One list holds CFG nodes (basic blocks) that we have
    // discovered might execute, and thus need to be processed,
    // and the other holds SSA nodes (instructions) that need
    // their "estimated" value to be updated.

    List<IRBlock*>  cfgWorkList;
    List<IRInst*>   ssaWorkList;

    // A key operation is to take an IR instruction and update
    // its "estimated" value on the lattice. This might happen when
    // we first discover the instruction could be executed, or
    // when we discover that one or more of its operands has
    // changed its lattice value so that we need to update our estimate.
    //
    void updateValueForInst(IRInst* inst)
    {
        // Block parameters are conceptually SSA "phi nodes", and it
        // doesn't make sense to update their values here, because the
        // actual candidate values for them comes from the predecessor blocks
        // that provide arguments. We will see that logic shortly, when
        // handling `IRUnconditionalBranch`.
        //
        if(as<IRParam>(inst))
            return;

        // We want to special-case terminator instructions here,
        // since abstract interpretation of them should cause blocks to
        // be marked as executed, etc.
        //
        if( auto terminator = as<IRTerminatorInst>(inst) )
        {
            if( auto unconditionalBranch = as<IRUnconditionalBranch>(inst) )
            {
                // When our abstract interpreter "executes" an unconditional
                // branch, it needs to mark the target block as potentially
                // executed. We do this by adding the target to our CFG work list.
                //
                auto target = unconditionalBranch->getTargetBlock();
                cfgWorkList.add(target);

                // Besides transferring control to another block, the other
                // thing our unconditional branch instructions do is provide
                // the arguments for phi nodes in the target block.
                // We thus need to interpret each argument on the branch
                // instruction like an "assignment" to the corresponding
                // parameter of the target block.
                //
                UInt argCount = unconditionalBranch->getArgCount();
                IRParam* pp = target->getFirstParam();
                for( UInt aa = 0; aa < argCount; ++aa, pp = pp->getNextParam() )
                {
                    IRInst* arg = unconditionalBranch->getArg(aa);
                    IRInst* param = pp;

                    // We expect the number of arguments and parameters to match,
                    // or else the IR is violating its own invariants.
                    //
                    SLANG_ASSERT(param);

                    // We will update the value for the target block's parameter
                    // using our "meet" operation (union of sets of possible values)
                    //
                    LatticeVal oldVal = getLatticeVal(param);

                    // If we've already determined that the block parameter could
                    // have any value whatsoever, there is no reason to bother
                    // updating it.
                    //
                    if(oldVal.flavor == LatticeVal::Flavor::Any)
                        continue;

                    // We can look up the lattice value for the argument,
                    // because we should have interpreted it already
                    //
                    LatticeVal argVal = getLatticeVal(arg);

                    // Now we apply the meet operation and see if the value changed.
                    //
                    LatticeVal newVal = meet(oldVal, argVal);
                    if( newVal != oldVal )
                    {
                        // If the "estimated" value for the parameter has changed,
                        // then we need to update it in our dictionary, and then
                        // make sure that all of the users of the parameter get
                        // their estimates updated as well.
                        //
                        setLatticeVal(param, newVal);
                        for( auto use = param->firstUse; use; use = use->nextUse )
                        {
                            ssaWorkList.add(use->getUser());
                        }
                    }
                }
            }
            else if( auto conditionalBranch = as<IRConditionalBranch>(inst) )
            {
                // An `IRConditionalBranch` is used for two-way branches.
                // We will look at the lattice value for the condition,
                // to see if we can narrow down which of the two ways
                // might actually be taken.
                //
                auto condVal = getLatticeVal(conditionalBranch->getCondition());

                // We do not expect to see a `None` value here, because that
                // would mean the user is branching based on an undefined
                // value.
                //
                // TODO: We should make sure there is no way for the user
                // to trigger this assert with bad code that involves
                // uninitialized variables. Right now we don't special
                // case the `undefined` instruction when computing lattice
                // values, so it shouldn't be a problem.
                //
                SLANG_ASSERT(condVal.flavor != LatticeVal::Flavor::None);

                // If the branch condition is a constant, we expect it to
                // be a Boolean constant. We won't assert that it is the
                // case here, just to be defensive.
                //
                if( condVal.flavor == LatticeVal::Flavor::Constant )
                {
                    if( auto boolConst = as<IRBoolLit>(condVal.value) )
                    {
                        // Only one of the two targe blocks is possible to
                        // execute, based on what we know of the condition,
                        // so we will add that target to our work list and
                        // bail out now.
                        //
                        auto target = boolConst->getValue() ? conditionalBranch->getTrueBlock() : conditionalBranch->getFalseBlock();
                        cfgWorkList.add(target);
                        return;
                    }
                }

                // As a fallback, if the condition isn't constant
                // (or somehow wasn't a Boolean constnat), we will
                // assume that either side of the branch could be
                // taken, so that both of the target blocks are
                // potentially executed.
                //
                cfgWorkList.add(conditionalBranch->getTrueBlock());
                cfgWorkList.add(conditionalBranch->getFalseBlock());
            }
            else if( auto switchInst = as<IRSwitch>(inst) )
            {
                // The handling of a `switch` instruction is similar to the
                // case for a two-way branch, with the main difference that
                // we have to deal with an integer condition value.

                auto condVal = getLatticeVal(switchInst->getCondition());
                SLANG_ASSERT(condVal.flavor != LatticeVal::Flavor::None);

                UInt caseCount = switchInst->getCaseCount();
                if( condVal.flavor == LatticeVal::Flavor::Constant )
                {
                    if( auto condConst = as<IRIntLit>(condVal.value) )
                    {
                        // At this point we have a constant integer condition
                        // value, and we just need to find the case (if any)
                        // that matches it. We will default to considering
                        // the `default` label as the target.
                        //
                        auto target = switchInst->getDefaultLabel();
                        for( UInt cc = 0; cc < caseCount; ++cc )
                        {
                            if( auto caseConst = as<IRIntLit>(switchInst->getCaseValue(cc)) )
                            {
                                if(caseConst->getValue() == condConst->getValue())
                                {
                                    target = switchInst->getCaseLabel(cc);
                                    break;
                                }
                            }
                        }

                        // Whatever single block we decided will get executed,
                        // we need to make sure it gets processed and then bail.
                        //
                        cfgWorkList.add(target);
                        return;
                    }
                }

                // The fallback is to assume that the `switch` instruction might
                // branch to any of its cases, or the `default` label.
                //
                for( UInt cc = 0; cc < caseCount; ++cc )
                {
                    cfgWorkList.add(switchInst->getCaseLabel(cc));
                }
                cfgWorkList.add(switchInst->getDefaultLabel());
            }

            // There are other cases of terminator instructions not handled
            // above (e.g., `return` instructions), but these can't cause
            // additional basic blocks in the CFG to execute, so we don't
            // need to consider them here.
            //
            // No matter what, we are done with a terminator instruction
            // after inspecting it, and there is no reason we have to
            // try and compute its "value."
            return;
        }

        // For an "ordinary" instruction, we will first check what value
        // has been registered for it already.
        //
        LatticeVal oldVal = getLatticeVal(inst);

        // If we have previous decided that the instruction could take
        // on any value whatsoever, then any further update to our
        // guess can't expand things more, and so there is nothing to do.
        //
        if( oldVal.flavor == LatticeVal::Flavor::Any )
        {
            return;
        }

        // Otherwise, we compute a new guess at the value of
        // the instruction based on the lattice values of the
        // stuff it depends on.
        //
        LatticeVal newVal = interpretOverLattice(inst);

        // If nothing changed about our guess, then there is nothing
        // further to do, because users of this instruction have
        // already computed their guess based on its current value.
        //
        if(newVal == oldVal)
        {
            return;
        }

        // If the guess did change, then we want to register our
        // new guess as the lattice value for this instruction.
        //
        setLatticeVal(inst, newVal);

        // Next we iterate over all the users of this instruction
        // and add them to our work list so that we can update
        // their values based on the new information.
        //
        for( auto use = inst->firstUse; use; use = use->nextUse )
        {
            ssaWorkList.add(use->getUser());
        }
    }

    // The `apply()` function will run the full algorithm.
    //
    void apply()
    {
        // We start with the busy-work of setting up our IR builder.
        //
        builderStorage.sharedBuilder = &shared->sharedBuilder;

        // We expect the caller to have filtered out functions with
        // no bodies, so there should always be at least one basic block.
        //
        auto firstBlock = code->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        // The entry block is always going to be executed when the
        // function gets called, so we will process it right away.
        //
        cfgWorkList.add(firstBlock);

        // The parameters of the first block are our function parameters,
        // and we want to operate on the assumption that they could have
        // any value possible, so we will record that in our dictionary.
        //
        for( auto pp : firstBlock->getParams() )
        {
            setLatticeVal(pp, LatticeVal::getAny());
        }

        // Now we will iterate until both of our work lists go dry.
        //
        while(cfgWorkList.getCount() || ssaWorkList.getCount())
        {
            // Note: there is a design choice to be had here
            // around whether we do `if if` or `while while`
            // for these nested checks. The choice can affect
            // how long things take to converge.

            // We will start by processing any blocks that we
            // have determined are potentially reachable.
            //
            while( cfgWorkList.getCount() )
            {
                // We pop one block off of the work list.
                //
                auto block = cfgWorkList[0];
                cfgWorkList.fastRemoveAt(0);

                // We only want to process blocks that haven't
                // already been marked as executed, so that we
                // don't do redundant work.
                //
                if( !isMarkedAsExecuted(block) )
                {
                    // We should mark this new block as executed,
                    // so we can ignore it if it ever ends up on
                    // the work list again.
                    //
                    markAsExecuted(block);

                    // If the block is potentially executed, then
                    // that means the instructions in the block are too.
                    // We will walk through the block and update our
                    // guess at the value of each instruction, which
                    // may in turn add other blocks/instructions to
                    // the work lists.
                    //
                    for( auto inst : block->getDecorationsAndChildren() )
                    {
                        updateValueForInst(inst);
                    }
                }
            }

            // Once we've cleared the work list of blocks, we
            // will start looking at individual instructions that
            // need to be updated.
            //
            while( ssaWorkList.getCount() )
            {
                // We pop one instruction that needs an update.
                //
                auto inst = ssaWorkList[0];
                ssaWorkList.fastRemoveAt(0);

                // Before updating the instruction, we will check if
                // the parent block of the instructin is marked as
                // being executed. If it isn't, there is no reason
                // to update the value for the instruction, since
                // it might never be used anyway.
                //
                IRBlock* block = as<IRBlock>(inst->getParent());

                // It is possible that an instruction ended up on
                // our SSA work list because it is a user of an
                // instruction in a block of `code`, but it is not
                // itself an instruction a block of `code`.
                //
                // For example, if `code` is an `IRGeneric` that
                // yields a function, then `inst` might be an
                // instruction of that nested function, and not
                // an instruction of the generic itself.
                // Note that in such a case, the `inst` cannot
                // possible affect the values computed in the outer
                // generic, or the control-flow paths it might take,
                // so there is no reason to consider it.
                //
                // We guard against this case by only processing `inst`
                // if it is a child of a block in the current `code`.
                //
                if(!block || block->getParent() != code)
                    continue;

                if( isMarkedAsExecuted(block) )
                {
                    // If the instruction is potentially executed, we update
                    // its lattice value based on our abstraction interpretation.
                    //
                    updateValueForInst(inst);
                }
            }
        }

        // Once the work lists are empty, our "guesses" at the value
        // of different instructions and the potentially-executed-ness
        // of blocks should have converged to a conservative steady state.
        //
        // We are now equiped to start using the information we've gathered
        // to modify the code.

        // First, we will walk through all the code and replace instructions
        // with constants where it is possible.
        //
        List<IRInst*> instsToRemove;
        for( auto block : code->getBlocks() )
        {
            for( auto inst : block->getDecorationsAndChildren() )
            {
                // We look for instructions that have a constnat value on
                // the lattice.
                //
                LatticeVal latticeVal = getLatticeVal(inst);
                if(latticeVal.flavor != LatticeVal::Flavor::Constant)
                    continue;

                // As a small sanity check, we won't go replacing an
                // instruction with itself (this shouldn't really come
                // up, since constants are supposed to be at the global
                // scope right now)
                //
                IRInst* constantVal = latticeVal.value;
                if(constantVal == inst)
                    continue;

                // We replace any uses of the instruction with its
                // constant expected value, and add it to a list of
                // instructions to be removed *iff* the instruction
                // is known to have no obersvable side effects.
                //
                inst->replaceUsesWith(constantVal);
                if( !inst->mightHaveSideEffects() )
                {
                    instsToRemove.add(inst);
                }
            }
        }

        // Once we've replaced the uses of instructions that evaluate
        // to constants, we make a second pass to remove the instructions
        // themselves (or at least those without side effects).
        //
        for( auto inst : instsToRemove )
        {
            inst->removeAndDeallocate();
        }

        // Next we are going to walk through all of the terminator
        // instructions on blocks and look for ones that branch
        // based on a constant condition. These will be rewritten
        // to use direct branching instructions, which will of course
        // need to be emitted using a builder.
        //
        auto builder = getBuilder();
        for( auto block : code->getBlocks() )
        {
            auto terminator = block->getTerminator();

            // We check if we have a `switch` instruction with a constant
            // integer as its condition.
            //
            if( auto switchInst = as<IRSwitch>(terminator) )
            {
                if( auto constVal = as<IRIntLit>(switchInst->getCondition()) )
                {
                    // We will select the one branch that gets taken, based
                    // on the constant condition value. The `default` label
                    // will of course be taken if no `case` label matches.
                    //
                    IRBlock* target = switchInst->getDefaultLabel();
                    UInt caseCount = switchInst->getCaseCount();
                    for(UInt cc = 0; cc < caseCount; ++cc)
                    {
                        auto caseVal = switchInst->getCaseValue(cc);
                        if(auto caseConst = as<IRIntLit>(caseVal))
                        {
                            if( caseConst->getValue() == constVal->getValue() )
                            {
                                target = switchInst->getCaseLabel(cc);
                                break;
                            }
                        }
                    }

                    // Once we've found the target, we will emit a direct
                    // branch to it before the old terminator, and then remove
                    // the old terminator instruction.
                    //
                    builder->setInsertBefore(terminator);
                    builder->emitBranch(target);
                    terminator->removeAndDeallocate();
                }
            }
            else if(auto condBranchInst = as<IRConditionalBranch>(terminator))
            {
                if( auto constVal = as<IRBoolLit>(condBranchInst->getCondition()) )
                {
                    // The case for a two-sided conditional branch is similar
                    // to the `switch` case, but simpler.

                    IRBlock* target = constVal->getValue() ? condBranchInst->getTrueBlock() : condBranchInst->getFalseBlock();

                    builder->setInsertBefore(terminator);
                    builder->emitBranch(target);
                    terminator->removeAndDeallocate();
                }
            
            }
        }

        // At this point we've replaced some conditional branches
        // that would always go the same way (e.g., a `while(true)`),
        // which should render some of our blocks unreachable.
        // We will collect all those unreachable blocks into a list
        // of blocks to be removed, and then go about trying to
        // remove them.
        //
        List<IRBlock*> unreachableBlocks;
        for( auto block : code->getBlocks() )
        {
            if( !isMarkedAsExecuted(block) )
            {
                unreachableBlocks.add(block);
            }
        }
        //
        // It might seem like we could just do:
        //
        //      block->removeAndDeallocate();
        //
        // for each of the blocks in `unreachableBlocks`, but there
        // is a subtle point that has to be considered:
        //
        // We have a structured control-flow representation where
        // certain branching instructions name "join points" where
        // control flow logically re-converges. It is possible that
        // one of our unreachable blocks is still being used as
        // a join point.
        //
        // For example:
        //
        //      if(A)
        //          return B;
        //      else
        //          return C;
        //      D;
        //
        // In the above example, the block that computes `D` is
        // unreachable, but it is still the join point for the `if(A)`
        // branch.
        //
        // Rather than complicate the encoding of join points to
        // try to special-case an unreachable join point, we will
        // instead retain the join point as a block with only a single
        // `unreachable` instruction.
        //
        // To detect which blocks are unreachable and unreferenced,
        // we will check which blocks have any uses. Of course, it
        // might be that some of our unreachable blocks still reference
        // one another (e.g., an unreachable loop) so we will start
        // by removing the instructions from the bodies of our unreachable
        // blocks to eliminate any cross-references between them.
        //
        for( auto block : unreachableBlocks )
        {
            // TODO: In principle we could produce a diagnostic here
            // if any of these unreachable blocks appears to have
            // "non-trivial" code in it (that is, any code explicitly
            // written by the user, and not just code synthesized by
            // the compiler to satisfy language rules). Making that
            // determination could be tricky, so for now we will
            // err on the side of allowing unreachable code without
            // a warning.
            //
            block->removeAndDeallocateAllDecorationsAndChildren();
        }
        //
        // At this point every one of our unreachable blocks is empty,
        // and there should be no branches from reachable blocks
        // to unreachable ones.
        //
        // We will iterate over our unreachable blocks, and process
        // them differently based on whether they have any remaining uses.
        //
        for( auto block : unreachableBlocks )
        {
            // At this point there had better be no edges branching to
            // our block. We determined it was unreachable, so there had
            // better not be branches from reachable blocks to this one,
            // and all the unreachable blocks had their instructions
            // removed, so there should be no branches to it from other
            // unreachable blocks (or itself).
            //
            SLANG_ASSERT(block->getPredecessors().isEmpty());

            // If the block is completely unreferenced, we can safely
            // remove and deallocate it now.
            //
            if( !block->hasUses() )
            {
                block->removeAndDeallocate();
            }
            else
            {
                // Otherwise, the block has at least one use (but
                // no predecessors), which should indicate that it
                // is an unreachable join point.
                //
                // We will keep the block around, but its entire
                // body will consist of a single `unreachable`
                // instruction.
                //
                builder->setInsertInto(block);
                builder->emitUnreachable();
            }
        }
    }
};

static void applySparseConditionalConstantPropagationRec(
    SharedSCCPContext*  shared,
    IRInst*             inst)
{
    if( auto code = as<IRGlobalValueWithCode>(inst) )
    {
        if( code->getFirstBlock() )
        {
            SCCPContext context;
            context.shared = shared;
            context.code = code;
            context.apply();
        }
    }

    for( auto childInst : inst->getDecorationsAndChildren() )
    {
        applySparseConditionalConstantPropagationRec(shared, childInst);
    }
}

void applySparseConditionalConstantPropagation(
    IRModule*       module)
{
    SharedSCCPContext shared;
    shared.module = module;
    shared.sharedBuilder.module = module;
    shared.sharedBuilder.session = module->getSession();

    applySparseConditionalConstantPropagationRec(&shared, module->getModuleInst());
}

}

