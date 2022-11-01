// slang-ir-dce.cpp
#include "slang-ir-dce.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
struct DeadCodeEliminationContext
{
    // This type implements a simple global DCE pass over
    // an entire module.
    //
    // We start with member variables to stand in for
    // the parameters that were passed to the top-level
    // `eliminateDeadCode` function.
    //
    IRModule*                       module;
    IRDeadCodeEliminationOptions    options;

    // If we removed an inst, there may be still "weak references" to the inst.
    // These uses will be replaced with `undefInst`.
    IRInst* undefInst = nullptr;

    // Our overall process is going to be to determine
    // which instructions in the module are "live"
    // and then eliminate anything that wasn't found to
    // be live.
    //
    // We will track the liveness state by keeping
    // a set of all instructions we have so far determined
    // to be live.
    //
    HashSet<IRInst*> liveInsts;

    // Querying whether an instruction has been
    // determined to be live is easy.
    //
    bool isInstLive(IRInst* inst)
    {
        // The only wrinkle is that we want to safeguard
        // against a null instruction (there are some
        // corner cases where we still construct IR
        // instructions with a null type).
        //
        if(!inst) return false;

        return liveInsts.Contains(inst);
    }

    // We are going to do an iterative analysis
    // where we mark instructions we know are
    // live, and then see if that can help us
    // identify any other instructions that
    // must also be live.
    //
    // For this, we will use a work list of
    // instructions that have been marked
    // as live, but for which we haven't
    // looked at their impact on other
    // instructions.
    //
    List<IRInst*> workList;

    // When we discover that an instruction seems
    // to be live, we will add it to our set,
    // and also the work list, but only if we
    // haven't done so previously.
    //
    void markInstAsLive(IRInst* inst)
    {
        // Again, we safeguard against null instructions
        // just in case.
        //
        if(!inst) return;

        if(liveInsts.Contains(inst))
            return;
        liveInsts.Add(inst);
        workList.add(inst);
    }

    IRInst* getUndefInst()
    {
        if (!undefInst)
        {
            for (auto inst : module->getModuleInst()->getChildren())
            {
                if (inst->getOp() == kIROp_undefined && inst->getDataType() && inst->getDataType()->getOp() == kIROp_VoidType)
                {
                    undefInst = inst;
                    break;
                }
            }
            if (!undefInst)
            {
                SharedIRBuilder builderStorage(module);
                IRBuilder builder(&builderStorage);
                builder.setInsertInto(module->getModuleInst());
                undefInst = builder.emitUndefined(builder.getVoidType());
            }
        }
        return undefInst;
    }

    // Given the basic infrastructrure above, let's
    // dive into the task of actually finding all
    // the live code in a module.
    //
    bool processModule()
    {
        // First of all, we know that the root module instruction
        // should be considered as live, because otherwise
        // we'd end up eliminating it, so that is a
        // good place to start.
        //
        markInstAsLive(module->getModuleInst());

        // Ensure there is a global undef inst that is always alive.
        // This undef inst will be used to fill in weak-referencing uses
        // whose used value is marked as dead and eliminated.
        // We always make sure this undef inst is available to prevent
        // infiniate oscilating loops.
        markInstAsLive(getUndefInst());

        // Marking the module as live should have
        // seeded our work list, so we can now start
        // processing entries off of our work list
        // until it goes dry.
        //
        while( workList.getCount() )
        {
            auto inst = workList.getLast();
            workList.removeLast();

            // At this point we know that `inst` is live,
            // and we want to start considering which other
            // instructions must be live because of that
            // knowlege.
            //
            // A first easy case is that the parent (if any)
            // of a live instruction had better be live, or
            // else we might delete the parent, and
            // the child with it.
            //
            markInstAsLive(inst->getParent());

            // Next the type of a live instruction, and all
            // of its operands must also be live, or else
            // we won't be able to compute its value.
            //
            markInstAsLive(inst->getFullType());
            UInt operandCount = inst->getOperandCount();
            for( UInt ii = 0; ii < operandCount; ++ii )
            {
                // There are some type of operands that needs to be treated as
                // "weak" references -- they can never hold things alive, and
                // whenever we delete the referenced value, these operands needs
                // to be replaced with `undef`.
                if (!isWeakReferenceOperand(inst, ii))
                    markInstAsLive(inst->getOperand(ii));
            }

            // Finally, we need to consider the children
            // and decorations of the instruction.
            //
            // Note that just because an instruction is
            // live doesn't mean its children must be, or
            // else we'd never eliminate *anything* (we
            // marked the whole module as live, and everything
            // is a transitive child of the module).
            //
            // Decorations, in contrast, are always live if their
            // parents are (because we don't want to silently drop
            // decorations). It is still important to *mark*
            // decorations as live, because they have operands,
            // and those operands need to be marked as live.
            // We will fold decorations into the same loop
            // as children for simplicity.
            //
            // To keep the code here simple, we'll defer the
            // decision of whether a child (or decoration)
            // should be live when its parent is to a subroutine.
            //
            for( auto child : inst->getDecorationsAndChildren() )
            {
                if(shouldInstBeLiveIfParentIsLive(child))
                {
                    // In this case, we know `inst` is live and
                    // its `child` should be live if its parent is,
                    // so the `child` must be live too.
                    //
                    markInstAsLive(child);
                }
            }
        }

        // If our work list runs dry, that means we've reached a steady
        // state where everything that is transitively relevant to
        // the "outputs" of the module has been marked as live.
        //
        // Now we can simply walk through all of our instructions
        // recursively and eliminate those that are "dead" by
        // virtue of not having been found live.
        //
        return eliminateDeadInstsRec(module->getModuleInst());
    }

    bool eliminateDeadInstsRec(IRInst* inst)
    {
        bool changed = false;
        // Given the instruction `inst` we need to eliminate
        // any dead code at, or under it.
        //
        // The easy case is if `inst` is dead (that is, not live).
        //
        if( !isInstLive(inst) )
        {
            // We can simply remove and deallocate `inst` because it is
            // dead, and not worry about any of its descendents,
            // because they must have been dead too (since we always
            // mark the parent of a live instruction as live).
            //
            if (inst->hasUses())
            {
                inst->replaceUsesWith(getUndefInst());
            }
            inst->removeAndDeallocate();
            changed = true;
        }
        else
        {
            // If `inst` is live, then we need to deal with the possibility
            // that its children/decorations (or descendents in general)
            // might still be dead.
            //
            // The biggest wrinkle is that we walk the linked list of
            // children/decorations a bit carefully, using a temporary
            // to hold the next node, in case we eliminate one of
            // the children as we go.
            //
            IRInst* next = nullptr;
            for( IRInst* child = inst->getFirstDecorationOrChild(); child; child = next )
            {
                next = child->getNextInst();
                changed |= eliminateDeadInstsRec(child);
            }
        }
        return changed;
    }

    // Now we come to the decision procedure we put off before:
    // should a given `inst` be live if its parent is?
    //
    bool shouldInstBeLiveIfParentIsLive(IRInst* inst)
    {
        return Slang::shouldInstBeLiveIfParentIsLive(inst, options);
    }
};

bool shouldInstBeLiveIfParentIsLive(IRInst* inst, IRDeadCodeEliminationOptions options)
{
    // The main source of confusion/complexity here is that
    // we are using the same routine to decide:
    //
    // * Should some ordinary instruction in a basic block be kept around?
    // * Should a basic block in some function be kept around?
    // * Should a function/type/variable in a module be kept around?
    //
    // Still, there are a few basic patterns we can observe.
    // First, if `inst` is an instruction that might have some effects
    // when it is executed, then we should keep it around.
    //
    if (inst->mightHaveSideEffects())
        return true;
    //
    // The `mightHaveSideEffects` query is conservative, and will
    // return `true` as its default mode, so once we are past that
    // query we know that `inst` is either something "structural"
    // (that makes up the program) rather than executable, or it
    // is executable but was on an allow-list of things that are
    // safe to eliminate.

    // Most top-level objects (functions, types, etc.) obviously
    // do *not* have side effects. That creates the risk that
    // we'll just go ahead and eliminate every single function/type
    // in a module. There needs to be a way to identify the
    // functions we want to keep around, and for right now
    // that is handled with the `[keepAlive]` decoration.
    //
    if (inst->findDecorationImpl(kIROp_KeepAliveDecoration))
        return true;
    //
    // We also consider anything with an `[export(...)]` as live,
    // when the appropriate option has been set.
    //
    // Note: our current approach to linking for back-end compilation
    // leaves many linakge decorations in place that we seemingly
    // don't need/want, so this option currently can't be enabled
    // unconditionally.
    //
    if (options.keepExportsAlive)
    {
        if (inst->findDecoration<IRExportDecoration>())
        {
            return true;
        }
    }

    if (options.keepLayoutsAlive && inst->findDecoration<IRLayoutDecoration>())
    {
        return true;
    }

    // A basic block is an interesting case. Knowing that a function
    // is live means that its entry block is live, but the liveness
    // of any other blocks is determined by whether they are referenced
    // by other instructions (e.g., a branch from one block to
    // another).
    //
    if (auto block = as<IRBlock>(inst))
    {
        // To determine whether this is the first block in its
        // parent function (or what-have-you) we can simply
        // check if there is a previous block before it.
        //
        auto prevBlock = block->getPrevBlock();
        return prevBlock == nullptr;
    }

    // There are a few special cases of "structural" instructions
    // that we don't want to eliminate, so we'll check for those next.
    //
    switch (inst->getOp())
    {
        // Function parameters obviously shouldn't get eliminated,
        // even if nothing references them, and block parameters
        // (phi nodes) will be considered live when their block is,
        // just so that we don't have to deal with any complications
        // around re-writing the relevant inter-block argument passing.
        //
        // TODO: A smarter DCE pass could deal with this case more
        // carefully, or we could improve the interprocedural SCCP
        // pass to deal with block parameters instead.
        //
    case kIROp_Param:
        return true;

        // IR struct types and witness tables are currently kludged
        // so that they have child instructions that represent their
        // entries (effectively `(key,value)` pairs), and those child
        // instructions are never directly referenced (e.g., an access
        // to a struct field references the *key* but not the `(key,value)`
        // pair that is the `IRField` instruction.
        //
        // TODO: at some point the IR should use a different representation
        // for struct types and witness tables that does away with
        // this problem.
        //
    case kIROp_StructField:
    case kIROp_WitnessTableEntry:
        return true;

    default:
        break;
    }

    // If none of the explicit cases above matched, then we will consider
    // the instruction to not be live just because its parent is. Further
    // analysis could still lead to a change in the status of `inst`, if
    // an instruction that uses it as an operand is marked live.
    //
    return false;
}

bool isWeakReferenceOperand(IRInst* inst, UInt operandIndex)
{
    // There are some type of operands that needs to be treated as
    // "weak" references -- they can never hold things alive, and
    // whenever we delete the referenced value, these operands needs
    // to be replaced with `undef`.
    switch (inst->getOp())
    {
    case kIROp_BoundInterfaceType:
        if (inst->getOperand(operandIndex)->getOp() == kIROp_WitnessTable)
            return true;
        break;
    case kIROp_SpecializationDictionaryItem:
        // Ignore all operands of SpecializationDictionaryItem.
        // This inst is used as a cache and shouldn't hold anything alive.
        return true;
    default:
        break;
    }
    return false;
}

// The top-level function for invoking the DCE pass
// is straighforward. We set up the context object
// and then defer to it for the real work.
//
bool eliminateDeadCode(
    IRModule*                           module,
    IRDeadCodeEliminationOptions const& options)
{
    DeadCodeEliminationContext context;
    context.module = module;
    context.options = options;

    return context.processModule();
}

}
