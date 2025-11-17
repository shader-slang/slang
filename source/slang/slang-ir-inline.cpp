// slang-ir-inline.cpp
#include "slang-ir-inline.h"

#include "../core/slang-performance-profiler.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"

// This file provides general facilities for inlining function calls.

//
// A  *call site* is an individual `call` instruction (`IRCall`), and the *callee*
// for a given call site is whatever is being called. When the callee is a `func`
// (`IRFunc`) or a specialization of a `generic` that yields a `func`, *and* the
// function has a body, then inlinling is possible.
//
// Different inlining passes may apply different heuristics or rules to decide
// which call sites should be inlined (if possible). The rules may be based
// on user-supplied hints, or on optimization criteria like performance and
// code size.

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
/// Base type for inlining passes, providing shared/common functionality
struct InliningPassBase
{
    /// The module that we are optimizing/transforming
    IRModule* m_module = nullptr;

    HashSet<IRInst*>* m_modifiedFuncs = nullptr;

    /// Initialize an inlining pass to operate on the given `module`
    InliningPassBase(IRModule* module)
        : m_module(module)
    {
    }

    /// Consider all the call sites in the module for inlining
    bool considerAllCallSites() { return considerAllCallSitesRec(m_module->getModuleInst()); }

    bool considerCallSiteInFunc(IRFunc* func)
    {
        bool result = false;

        // Repeat until we run out of callees to inline.
        for (;;)
        {
            bool changed = false;

            // Collect all the call sites in the function.
            List<IRCall*> callsites;
            for (auto block : func->getBlocks())
            {
                for (auto inst : block->getChildren())
                {
                    if (auto call = as<IRCall>(inst))
                    {
                        callsites.add(call);
                    }
                }
            }

            // Consider each call site.
            for (auto call : callsites)
            {
                changed |= considerCallSite(call);
            }
            result |= changed;
            if (!changed)
                break;
        }
        return result;
    }

    /// Consider all call sites at or under `inst` for inlining
    bool considerAllCallSitesRec(IRInst* inst)
    {
        bool changed = false;

        if (auto func = as<IRFunc>(inst))
        {
            changed = considerCallSiteInFunc(func);
        }
        else if (auto call = as<IRCall>(inst))
        {
            considerCallSite(call);
        }

        // Recursively consider the children of inst.
        for (auto child : inst->getModifiableChildren())
        {
            changed |= considerAllCallSitesRec(child);
        }
        return changed;
    }

    // In order to inline a call site, we need certain information
    // to be present/available. Most notable is that the callee must
    // be known, and it must be in the form of an `IRFunc`.
    //
    // Since checking whether we *can* inline a call site involves
    // finding all of this information, we will use that opportunity
    // to package it all up in a `struct` that can be re-used when
    // we actually get around to inlining a call site.

    /// Information about a call site to be inlined
    struct CallSiteInfo
    {
        /// The call instruction.
        IRCall* call = nullptr;

        /// The function being called.
        ///
        /// For an inlinable call, this must be non-null and a valid function *definition* (with a
        /// body) for inlining to proceed.
        IRFunc* callee = nullptr;

        /// The specialization of the function, if any.
        ///
        /// For an inlineable call, this must be non-null if the function is generic, but may be
        /// null otherwise.
        IRSpecialize* specialize = nullptr;

        /// The generic being specialized.
        ///
        /// For an inlineable call, this must be be non-null if `specialize` is non-null.
        IRGeneric* generic = nullptr;
    };

    // With `CallSiteInfo` defined, we can now understand the
    // basic proces of considering a call site for inlining.

    /// Consider the given `call` site, and possibly inline it.
    bool considerCallSite(IRCall* call)
    {
        // We start by checking if inlining would even be possible,
        // since doing so collects information about the call site
        // that can simplify the following steps.
        //
        // If the call can't be inlined, there is nothing else
        // to consider and we bail out.
        //
        CallSiteInfo callSite;
        if (!canInline(call, callSite))
            return false;

        // If we've decided that we *can* inline the given call
        // site, we next need to check if we *should*. The rules
        // for when we should inline may vary by subclass,
        // so `shouldInline` is a virtual method.
        //
        if (!shouldInline(callSite))
            return false;

        // Finally, if we both *can* and *should* inline the
        // given call site, we hand off the a worker routine
        // that does the meat of the work.
        //
        if (m_modifiedFuncs)
        {
            if (auto parentFunc = getParentFunc(call))
                m_modifiedFuncs->add(parentFunc);
        }
        inlineCallSite(callSite);
        return true;
    }

    // Every subclas of `InliningPassBase` should provide its own
    // definition of `shouldInline`. We define a default implementation
    // here for the benefit of passes that might implement their
    // own logic for deciding what to inline, bypassing `considerCallSite`.

    /// Determine whether `callSite` should be inlined.
    virtual bool shouldInline(CallSiteInfo const& callSite)
    {
        SLANG_UNUSED(callSite);
        return false;
    }

    static bool hasGenericAsmInst(IRInst* func)
    {
        auto f = as<IRFunc>(getResolvedInstForDecorations(func));
        if (!f)
            return false;
        for (auto b : f->getBlocks())
        {
            if (as<IRGenericAsm>(b->getTerminator()))
                return true;
        }
        return false;
    }

    /// Determine whether `call` can be inlined, and if so write information about it to
    /// `outCallSite`
    bool canInline(IRCall* call, CallSiteInfo& outCallSite)
    {
        // We can start by writing the `call` instruction into our `CallSiteInfo`.
        //
        outCallSite.call = call;

        // Next we consider the callee.
        //
        IRInst* callee = call->getCallee();

        // If the callee is a `specialize` instruction, then we
        // want to look at what is being specialized instead.
        //
        if (auto specialize = as<IRSpecialize>(callee))
        {
            // If the `specialize` is applied to something other
            // than a `generic` instruction, then we can't
            // inline the call site. This can happen for a
            // call to a generic method in an interface.
            //
            IRGeneric* generic = findSpecializedGeneric(specialize);
            if (!generic)
                return false;

            // If we have a `generic` instruction, then we
            // will look to see if we can determine what
            // it returns. If a result is found, that
            // will be used as the new callee for this
            // call site.
            //
            // If we can't identify the value that the generic
            // yields, then inlining isn't possible.
            //
            callee = findGenericReturnVal(generic);
            if (!callee)
                return false;

            // If we decide to inline this call, then the information
            // we've just extracted about generic specialization
            // will be relevant, so we write it to the `CallSiteInfo` now.
            //
            outCallSite.specialize = specialize;
            outCallSite.generic = generic;
        }

        // Once we've dispensed with any possible generic specialization
        // we will check if the callee is a `func` instruction (`IRFunc`).
        //
        // If it is not, then inlining isn't possible.
        //
        auto calleeFunc = as<IRFunc>(callee);
        if (!calleeFunc)
            return false;
        //
        // If the callee *is* a function, then we can update
        // the `CalleSiteInfo` with what we've found.
        //
        outCallSite.callee = calleeFunc;

        for (auto decor : callee->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_IntrinsicOpDecoration:
                return true;
            }
        }

        // We cannot inline a function that is defined by a generic asm inst.
        if (hasGenericAsmInst(callee))
            return false;

        // At this point the `CallSiteInfo` is complete and
        // could be used for inlining, but we have additional
        // checks to make.
        //
        // In particular, we should only go about inlining
        // a call site if the callee function is a full definition
        // in the IR (not just a declaration).
        //
        if (!isDefinition(calleeFunc))
            return false;

        // We cannot inline a call inside an `IRExpand`.
        // Because this will make the cfg inside the `IRExpand` too complex,
        // and our expand specialization logic isn't general enough to deal
        // with that yet.
        for (auto parent = call->getParent(); parent; parent = parent->getParent())
        {
            if (as<IRExpand>(parent))
                return false;
            if (as<IRGlobalValueWithCode>(parent))
                break;
        }
        return true;
    }

    // Find an existing debug function for the given function
    IRInst* findExistingDebugFunc(IRFunc* func)
    {
        if (auto debugFuncDecor = func->findDecoration<IRDebugFuncDecoration>())
        {
            return debugFuncDecor->getDebugFunc();
        }
        return nullptr;
    }

    struct DebugInlineInfo
    {
        IRInst* newDebugInlinedAt = nullptr;
        IRInst* calleeDebugFunc = nullptr;
    };

    // Sets up the initial debug information structures required *before* inlining a call site.
    //
    // This function performs the following steps:
    // 1. Checks if the callee function has associated debug location information.
    // 2. Obtain the call inst's existing debug information.
    // 3. Finds the last `IRDebugLine` preceding the `call` instruction to determine the source
    //    location (line, col, file) of the call site.
    // 4. Emits an `IRDebugInlinedAt` instruction with outer set to callDebugInlinedAt.
    // 5. Inserts the newly created `IRDebugInlinedAt` instruction immediately *before* the `call`
    // instruction.
    DebugInlineInfo emitCalleeDebugInlinedAt(IRCall* call, IRFunc* callee, IRBuilder& builder)
    {
        IRDebugLine* lastDebugLine = nullptr;

        if (!callee->findDecoration<IRDebugLocationDecoration>())
        {
            return DebugInlineInfo();
        }

        // Check if the call inst is part of an existing scope. If yes, then we restore
        // that scope after the inlining of callee. This case can occur when we have out of
        // order inlining. See forceinline-basic-block-inline-order.slang test for that use
        // case. If we are travesing back the call inst and if we find a DebugNoScope, it means
        // that there's another function that was inlined. We don't want that scope. If the call
        // inst truly belongs to another DebugScope, then we should hit a DebugScope inst
        // *before* we see a DebugNoScope
        IRDebugScope* callDebugScope = nullptr;
        builder.setInsertAfter(call);
        for (IRInst* inst = call->getPrevInst(); inst; inst = inst->getPrevInst())
        {
            if (as<IRDebugNoScope>(inst))
            {
                break;
            }
            if (as<IRDebugScope>(inst))
            {
                callDebugScope = as<IRDebugScope>(inst);
                builder.emitDebugScope(callDebugScope->getScope(), callDebugScope->getInlinedAt());
                break;
            }
        }
        if (!callDebugScope)
        {
            builder.emitDebugNoScope();
        }

        IRDebugInlinedAt* callDebugInlinedAt = nullptr;
        for (IRInst* inst = call->getPrevInst(); inst; inst = inst->getPrevInst())
        {
            if (as<IRDebugNoScope>(inst))
            {
                break;
            }
            if (as<IRDebugInlinedAt>(inst))
            {
                callDebugInlinedAt = as<IRDebugInlinedAt>(inst);
                break;
            }
        }

        // Find the last IRDebugLine to extract debug info.
        for (IRInst* inst = call->getPrevInst(); inst; inst = inst->getPrevInst())
        {
            if (auto debugLine = as<IRDebugLine>(inst))
            {
                lastDebugLine = debugLine;
                break;
            }
        }

        if (!lastDebugLine)
            return DebugInlineInfo();

        auto calleeDebugFunc = findExistingDebugFunc(callee);

        // The caller func is the right lexical scope needed for nsight to show where
        // the function is getting inlined inside.
        if (auto callerFunc = getParentFunc(call))
        {
            // When `maybeAddDebugLocationDecoration()` failed to find the source
            // location, IRDebugFuncDecoration is expected to be absent.
            if (auto callerDebugFunc = findExistingDebugFunc(callerFunc))
            {
                builder.setInsertBefore(call);
                auto newDebugInlinedAt = builder.emitDebugInlinedAt(
                    lastDebugLine->getLineStart(),
                    lastDebugLine->getColStart(),
                    lastDebugLine->getSource(),
                    callerDebugFunc,
                    callDebugInlinedAt);

                return DebugInlineInfo{newDebugInlinedAt, calleeDebugFunc};
            }
            else
            {
                // It is more likely to be a bug when IRDebugLocationDecoration exists and
                // IRDebugFuncDecoration doesn't exist.
                SLANG_ASSERT(nullptr == callerFunc->findDecoration<IRDebugLocationDecoration>());
            }
        }

        return DebugInlineInfo();
    }

    /// Inline the given `callSite`, which is assumed to have been validated
    void inlineCallSite(CallSiteInfo const& callSite)
    {
        // Information about the call site, including
        // the `call` instruction and the callee `func`
        // should already have been computed and stored
        // in the `CallSiteInfo`.
        //
        IRCall* call = callSite.call;
        IRFunc* callee = callSite.callee;

        // We will use the existing IR cloning infrastructure to clone
        // the body of the callee, but we need to establish an
        // environment for cloning in which any parameters of
        // the callee are replaced with the matching arguments
        // at the call site.
        //
        IRCloneEnv env;

        // We also need an `IRBuilder` to construct the cloned IR,
        // and will set it up to insert before the `call` that
        // is going to be replaced.
        //
        IRBuilder builder(m_module);
        builder.setInsertBefore(call);

        // If callee is an intrinsic op, just issue that intrinsic and be done.
        if (auto intrinsicOpDecor = callee->findDecoration<IRIntrinsicOpDecoration>())
        {
            List<IRInst*> args;
            for (UInt i = 0; i < call->getArgCount(); i++)
                args.add(call->getArg(i));
            auto op = intrinsicOpDecor->getIntrinsicOp();
            if (op == kIROp_Nop)
            {
                SLANG_RELEASE_ASSERT(call->getArgCount() >= 1);
                call->replaceUsesWith(call->getArg(0));
            }
            else
            {
                auto newCall = builder.emitIntrinsicInst(
                    call->getFullType(),
                    op,
                    args.getCount(),
                    args.getBuffer());
                call->replaceUsesWith(newCall);
            }
            call->removeAndDeallocate();
            return;
        }

        // If the callee is a generic function, then we will
        // need to include the substitution of generic parameters
        // with their argument values in our cloning.
        //
        if (auto specialize = callSite.specialize)
        {
            auto generic = callSite.generic;

            // We start by establishing a mapping from the
            // generic parameters to the matching arguments.
            //
            Int argCounter = 0;
            for (auto param : generic->getParams())
            {
                SLANG_ASSERT(argCounter < (Int)specialize->getArgCount());
                auto arg = specialize->getArg(argCounter++);

                env.mapOldValToNew.add(param, arg);
            }
            SLANG_ASSERT(argCounter == (Int)specialize->getArgCount());

            // We also need to clone any instructions in the
            // body of the `generic` being specialized, since
            // these might construct types or constants that
            // reference the generic parameters.
            //
            auto body = generic->getFirstBlock();
            SLANG_ASSERT(!body->getNextBlock()); // All IR generics should have a single block.

            for (auto inst : body->getChildren())
            {
                if (inst == callee)
                {
                    // We don't want to create a clone of the callee
                    // function at the call site, since it would
                    // immediately become dead code when we inline
                    // its body.
                }
                else if (as<IRReturn>(inst))
                {
                    // We also don't want to clone any `return`
                    // instruction in the generic, since that is
                    // how they yield their result (which we
                    // already know is `callee`.
                }
                else
                {
                    // In the default case, we just clone the instruction
                    // from the body of the generic into the call site.
                    //
                    // TODO: This assumes that deduplication will work
                    // as intended, so in practice we might run into
                    // problems if we create new instances of IR types
                    // or constants that already exist.
                    //
                    cloneInst(&env, &builder, inst);
                }
            }
        }

        // Compared to dealing with generic parameters, the process
        // for dealing with value parameters is much simpler.
        //
        {
            // For each parameter of the callee function, we
            // insert a mapping into `env` from that parameter to the
            // matching argument at the call site.
            //
            Int argCounter = 0;
            for (auto param : callee->getParams())
            {
                SLANG_ASSERT(argCounter < (Int)call->getArgCount());
                auto arg = call->getArg(argCounter++);
                env.mapOldValToNew.add(param, arg);
            }
            SLANG_ASSERT(argCounter == (Int)call->getArgCount());
        }

        inlineFuncBody(callSite, &env, &builder);
    }

    // When instructions are cloned, with cloneInst no sourceLoc information is copied over by
    // default. Here we attempt some policy about copying sourceLocs when inlining.
    //
    // An assumption here is that [__unsafeForceInlineEarly] will not be in user code (when we have
    // more general inlining this will not follow).
    //
    // Therefore we probably *don't* want to copy sourceLoc from the original definition in the core
    // module because
    //
    // * That won't be much use to the user (they can't easily see the core module code currently
    // for example)
    // * That the definitions in the core module are currently 'mundane' and largely exist to flesh
    // out language features - such that
    //   their being in the core module would likely be surprising to users
    //
    // That being the case, we actually copy the call sites sourceLoc if it's defined, and only fall
    // back onto the originating loc, if that's not defined.
    //
    // We *could* vary behavior if we knew if the function was defined in the core module. There
    // doesn't appear to be a decoration for this. We could find out by looking at the source loc
    // and checking if it's in the range of the core module - this would actually be a fast and easy
    // but to do properly this way you'd want a way to mark that source range that would also work
    // across serialization.
    //
    // For now this punts on this, and just assumes [__unsafeForceInlineEarly] is not in user code.
    static void _setSourceLoc(IRInst* clonedInst, IRInst* srcInst, CallSiteInfo const& callSite)
    {
        SourceLoc sourceLoc;

        if (callSite.call->sourceLoc.isValid())
        {
            // Default to using the source loc at the call site
            sourceLoc = callSite.call->sourceLoc;
        }
        else if (srcInst->sourceLoc.isValid())
        {
            // If we don't have that copy the inst being cloned sourceLoc
            sourceLoc = srcInst->sourceLoc;
        }

        clonedInst->sourceLoc = sourceLoc;
    }
    static IRInst* _cloneInstWithSourceLoc(
        CallSiteInfo const& callSite,
        IRCloneEnv* env,
        IRBuilder* builder,
        IRInst* inst)
    {
        IRInst* clonedInst = cloneInst(env, builder, inst);
        _setSourceLoc(clonedInst, inst, callSite);
        return clonedInst;
    }

    /// Inline the body of the callee for `callSite`, for a callee that has only
    /// a single basic block.
    ///
    void inlineSingleBlockFuncBody(
        CallSiteInfo const& callSite,
        IRCloneEnv* env,
        IRBuilder* builder,
        IRInst* newDebugInlinedAt,
        IRInst* calleeDebugFunc)
    {
        auto call = callSite.call;
        auto callee = callSite.callee;

        // The callee had better have only a single basic block.
        //
        auto firstBlock = callee->getFirstBlock();
        SLANG_ASSERT(!firstBlock->getNextBlock());

        // We will loop over the instructions in the block and clone
        // them into the same basic block as the `call`.
        //
        builder->setInsertBefore(call);
        IRInst* calleeDebugScope = nullptr;
        if (calleeDebugFunc && newDebugInlinedAt)
        {
            calleeDebugScope = builder->emitDebugScope(calleeDebugFunc, newDebugInlinedAt);
        }

        // Along the way, we will detect any `return` instruction,
        // and remember the (clone of the) returned value.
        //
        IRInst* returnVal = nullptr;
        List<IRDebugInlinedAt*> debugInlinedInsts;
        for (auto inst : firstBlock->getChildren())
        {
            switch (inst->getOp())
            {
            default:
                _cloneInstWithSourceLoc(callSite, env, builder, inst);
                break;

            case kIROp_Param:
                // Parameters of the first block are the parameters of
                // the function itself, so we skip them rather than
                // clone them.
                //
                break;

            case kIROp_Return:
                // We expect to see only a single `return` instruction,
                // and when we see it we note the value being returned.
                //
                SLANG_ASSERT(!returnVal);
                returnVal = findCloneForOperand(env, inst->getOperand(0));
                break;

            case kIROp_DebugNoScope:
                {
                    if (calleeDebugScope)
                        _cloneInstWithSourceLoc(callSite, env, builder, calleeDebugScope);
                    break;
                }

            case kIROp_DebugInlinedAt:
                {
                    auto clonedInst = _cloneInstWithSourceLoc(callSite, env, builder, inst);
                    if (!as<IRDebugInlinedAt>(clonedInst)->isOuterInlinedPresent())
                        debugInlinedInsts.add(as<IRDebugInlinedAt>(clonedInst));
                    break;
                }
            }
        }
        // For any debugInlinedAt without an outerinlinedAt, emit a new debugInlinedAt with the
        // outer set, and delete the older debugInlinedAt
        for (auto inst : debugInlinedInsts)
        {
            if (newDebugInlinedAt && !inst->isOuterInlinedPresent())
            {
                builder->setInsertAfter(inst);
                auto newInlinedAt = builder->emitDebugInlinedAt(
                    inst->getLine(),
                    inst->getCol(),
                    inst->getFile(),
                    inst->getDebugFunc(),
                    newDebugInlinedAt);
                inst->replaceUsesWith(newInlinedAt);
                inst->removeAndDeallocate();
            }
        }
        // We are going to remove the original `call` now that the callee
        // has been inlined, but before we do that we need to replace
        // all uses of the `call` with whatever value was produced by the
        // inlined body of the callee.
        //
        if (returnVal)
        {
            call->replaceUsesWith(returnVal);
        }
        else
        {
            call->replaceUsesWith(builder->getVoidValue());
        }

        // Once the `call` has no uses, we can safely remove it.
        //
        call->removeAndDeallocate();
    }

    /// Inline the body of the callee for `callSite`.
    // Here is the algorithm for inserting debug information for slang inlined functions:
    // 1. Check if the call inst belongs to an existing debug scope and find corresponding
    // debugInlinedAt. [callDebugScope, callDebugInlinedAt]
    //    1a. If callDebugScope exists, emit this debug Scope* after* the call inst.
    // 2. Emit a new DebugInlinedAt inst, with debugFunc of the callee, and outer debugInlinedAt is
    // callDebugInlinedAt. [newDebugInlinedAt]
    //    2a. If calleDebugScope does not exist, emit debugNoScope after the call inst.
    // 3. Clone the callee body.
    // 4. For each cloned block, do this:
    //    4a.Emit a new DebugScope inst setting the current scope to newDebugInlinedAt.
    //    [calleeDebugScope] 4b.Emit a DebugNoScope at the end of each block. 4c.If callDebugScope
    //    exists, do not emit a DebugNoScope for the last block.
    // 5. For each cloned debugInlinedAt inst, if its outer inlined at operand is null, set it to
    // the new DebugInlinedAt inst inserted at the top of the block.
    // 6. For each cloned debugNoScope inst, replace it with calleeDebugScope. (This is because all
    // cloned insts are in callee's scope).
    void inlineFuncBody(CallSiteInfo const& callSite, IRCloneEnv* env, IRBuilder* builder)
    {
        auto callee = callSite.callee;
        auto call = callSite.call;

        auto debugInlineInfo = emitCalleeDebugInlinedAt(call, callee, *builder);

        // Collect all arguments that are pointers, so we can propagate their address
        // spaces to the cloned instructions after inlining.
        List<IRInst*> ptrArgList;
        for (UInt i = 0; i < call->getArgCount(); i++)
        {
            auto arg = call->getArg(i);
            if (as<IRPtrTypeBase>(arg->getDataType()))
                ptrArgList.add(arg);
        }

        // If the callee consists of a single basic block *and* that block
        // ends with a `return` instruction, then we can apply a simple approach
        // to inlining that is compatible with any call site (including those
        // at the global scope).
        //
        auto firstBlock = callee->getFirstBlock();
        SLANG_ASSERT(firstBlock);
        if (!firstBlock->getNextBlock() && as<IRReturn>(firstBlock->getTerminator()))
        {
            inlineSingleBlockFuncBody(
                callSite,
                env,
                builder,
                debugInlineInfo.newDebugInlinedAt,
                debugInlineInfo.calleeDebugFunc);
        }
        else
        {
            // If the callee has multiple blocks, use the more complex inlining approach
            inlineMultipleBlockFuncBody(
                callSite,
                env,
                builder,
                debugInlineInfo.newDebugInlinedAt,
                debugInlineInfo.calleeDebugFunc);
        }

        // Propagate the address space from the argument to the cloned instructions.
        propagateAddressSpaceFromInsts(_Move(ptrArgList));
    }

    // Inline the body of the callee for `callSite`, for a callee that has multiple basic blocks.
    void inlineMultipleBlockFuncBody(
        CallSiteInfo const& callSite,
        IRCloneEnv* env,
        IRBuilder* builder,
        IRInst* newDebugInlinedAt,
        IRInst* calleeDebugFunc)
    {
        auto call = callSite.call;
        auto callee = callSite.callee;

        // If the callee has any non-trivial control flow (multiple basic blocks
        // and terminators other than `return`), we will need to split the control
        // flow of the caller at the block that contains `call`.
        //
        // For any of this to work, we have to assume that the `call` appears
        // in a basic block inside of a function (not, e.g., at the global scope).
        //
        auto callerBlock = callSite.call->getParent();
        SLANG_ASSERT(as<IRBlock>(callerBlock));
        auto callerFunc = callerBlock->getParent();
        SLANG_ASSERT(callerFunc);

        // As a fail-safe for release builds, if the above expectations are somehow
        // *not* met, we will fall back to not inlining the call at all.
        //
        if (!callerFunc)
        {
            return;
        }

        // We will create a new basic block block in the parent function that
        // will contain all the instructions that come *after* the `call`.
        //
        builder->setInsertInto(callerFunc);
        auto afterBlock = builder->createBlock();

        // Many operations (e.g. `cloneInst`) has define-before-use assumptions on the IR.
        // It is important to make sure we keep the ordering of blocks by inserting the
        // second half of the basic block right after `callerBlock`.
        afterBlock->insertAfter(callerBlock);
        afterBlock->sourceLoc = callSite.call->getNextInst()->sourceLoc;
        // Define a param in afterBlock to receive the return value from the call.
        builder->setInsertInto(afterBlock);

        IRInst* returnValParam = nullptr;
        if (callSite.call->getDataType()->getOp() != kIROp_VoidType)
            returnValParam = builder->emitParam(callSite.call->getDataType());

        // Move all insts after the call in `callerBlock` to `afterBlock`.
        {
            auto inst = callSite.call->getNextInst();
            while (inst)
            {
                auto next = inst->getNextInst();
                inst->removeFromParent();
                inst->insertAtEnd(afterBlock);
                inst = next;
            }
        }

        List<IRBlock*> clonedBlocks;
        for (auto calleeBlock : callee->getBlocks())
        {
            auto clonedBlock = builder->createBlock();
            clonedBlock->insertBefore(afterBlock);
            _setSourceLoc(clonedBlock, calleeBlock, callSite);
            env->mapOldValToNew[calleeBlock] = clonedBlock;
        }

        // Insert a branch into the cloned first block at the end of `callerBlock`.
        builder->setInsertInto(callerBlock);
        auto mainBlock = as<IRBlock>(env->mapOldValToNew.getValue(callee->getFirstBlock()));
        auto newBranch = builder->emitLoop(mainBlock, afterBlock, mainBlock);
        _setSourceLoc(newBranch, call, callSite);

        // Clone all basic blocks over to the call site.
        bool isFirstBlock = true;
        for (auto calleeBlock : callee->getBlocks())
        {
            auto clonedBlock = env->mapOldValToNew.getValue(calleeBlock);
            builder->setInsertInto(clonedBlock);
            // We will loop over the instructions of the each block,
            // and clone each of them appropriately.
            //
            for (auto inst : calleeBlock->getChildren())
            {
                if (inst->getOp() == kIROp_Param)
                {
                    // Parameters in the first block can be completely ignored
                    // because they have all been replaced via `env`.
                    if (isFirstBlock)
                    {
                        continue;
                    }
                }

                switch (inst->getOp())
                {
                default:
                    // The default value is to clone the instruction using
                    // the existing cloning infrastructure and the `env`
                    // we have already set up.
                    //
                    // SourceLoc information is copied if there is appropriate data available.
                    _cloneInstWithSourceLoc(callSite, env, builder, inst);
                    break;

                case kIROp_Return:
                    // A return is replaced with a branch into `afterBlock`
                    // to return the control flow to the location after the original `call`.
                    // We also need to note the (clone of the) value being
                    // returned, so that we can use it to replace the value
                    // of the original call.
                    //
                    {
                        auto returnedValue = findCloneForOperand(env, inst->getOperand(0));
                        auto returnBranch =
                            builder->emitBranch(afterBlock, returnValParam ? 1 : 0, &returnedValue);
                        _setSourceLoc(returnBranch, inst, callSite);
                    }
                    break;
                }
            }
            isFirstBlock = false;
        }
        // For each existing debugNoScope inst, replace it with new debug scope we emit.
        // For any debugInlinedAt without an outerinlinedAt, emit a new debugInlinedAt with the
        // outer set, and delete the older debugInlinedAt
        if (newDebugInlinedAt && callee->findDecoration<IRDebugLocationDecoration>())
        {
            for (auto calleeBlock : callee->getBlocks())
            {
                IRBlock* clonedBlock = as<IRBlock>(env->mapOldValToNew.getValue(calleeBlock));
                setInsertBeforeOrdinaryInst(builder, clonedBlock->getFirstOrdinaryInst());
                builder->emitDebugScope(calleeDebugFunc, newDebugInlinedAt);

                List<IRInst*> debugNoScopeToRemove;
                List<IRDebugInlinedAt*> debugInlinedAtToProcess;
                for (auto inst : clonedBlock->getChildren())
                {
                    if (as<IRDebugNoScope>(inst))
                    {
                        debugNoScopeToRemove.add(inst);
                    }
                    if (auto inlinedAt = as<IRDebugInlinedAt>(inst))
                    {
                        if (!inlinedAt->isOuterInlinedPresent())
                        {
                            debugInlinedAtToProcess.add(inlinedAt);
                        }
                    }
                }
                for (auto inst : debugNoScopeToRemove)
                {
                    builder->setInsertAfter(inst);
                    builder->emitDebugScope(calleeDebugFunc, newDebugInlinedAt);
                    inst->removeAndDeallocate();
                }
                for (auto inlinedAt : debugInlinedAtToProcess)
                {
                    builder->setInsertAfter(inlinedAt);
                    auto newInlinedAt = builder->emitDebugInlinedAt(
                        inlinedAt->getLine(),
                        inlinedAt->getCol(),
                        inlinedAt->getFile(),
                        inlinedAt->getDebugFunc(),
                        newDebugInlinedAt);
                    inlinedAt->replaceUsesWith(newInlinedAt);
                    inlinedAt->removeAndDeallocate();
                }
            }
        }
        // If there was a `returnVal` instruction that established
        // the return value of the inlined function, then that value
        // should be used to replace any uses of the original call.
        //
        if (returnValParam)
        {
            call->replaceUsesWith(returnValParam);
        }
        else
        {
            call->replaceUsesWith(builder->getVoidValue());
        }

        // Once we've cloned the body of the callee in at the call site,
        // there is no reason to keep around the original `call` instruction,
        // so we remove it.
        //
        call->removeAndDeallocate();
    }
};

/// An inlining pass that inlines calls to `[unsafeForceInlineEarly]` functions
struct MandatoryEarlyInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    MandatoryEarlyInliningPass(IRModule* module)
        : Super(module)
    {
    }

    bool shouldInline(CallSiteInfo const& info)
    {
        if (info.callee->findDecoration<IRIntrinsicOpDecoration>())
            return true;

        if (info.callee->findDecoration<IRUnsafeForceInlineEarlyDecoration>())
            return true;
        return false;
    }
};


bool performMandatoryEarlyInlining(IRModule* module, HashSet<IRInst*>* modifiedFuncs)
{
    SLANG_PROFILE;

    MandatoryEarlyInliningPass pass(module);
    pass.m_modifiedFuncs = modifiedFuncs;
    return pass.considerAllCallSites();
}

namespace
{ // anonymous

// Inlines calls that involve String types
struct TypeInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    TargetProgram* targetProgram;

    bool shouldInlineBorrowRefTypesForSPIRV = false;

    TypeInliningPass(IRModule* module, TargetProgram* inTargetProgram)
        : Super(module), targetProgram(inTargetProgram)
    {
        if (targetProgram->shouldEmitSPIRVDirectly())
        {
            shouldInlineBorrowRefTypesForSPIRV = true;
        }
    }

    bool doesTypeRequireInline(IRType* type, IRInst* arg, IRFunc* callee)
    {
        // TODO(JS):
        // I guess there is a question here about what type around string requires
        // inlining.
        // For example if we had an array of strings etc.
        // For now we just consider just basic string types.
        const auto op = type->getOp();
        switch (op)
        {
        case kIROp_RefParamType:
            {
                if (callee->findDecoration<IRNoRefInlineDecoration>())
                    return false;
                return true;
            }
        case kIROp_BorrowInParamType:
        case kIROp_BorrowInOutParamType:
            {
                if (shouldInlineBorrowRefTypesForSPIRV)
                {
                    if (!arg)
                        return true;

                    // If the argument is a refernece to a local variable "memory object",
                    // we don't need to inline.
                    auto rootAddr = getRootAddr(arg);
                    if (as<IRVar>(rootAddr) || as<IRGlobalVar>(rootAddr))
                        return false;

                    // Otherwise, we should inline the function before emitting SPIR-V,
                    // because:
                    // 1) SPIRV does not support expressing a function that takes a pointer
                    // parameter whose address space is not Function or Private.
                    // 2) SPIRV does not allow passing a pointer-typed argument that isn't a direct
                    // reference to a variable "memory object". For example, you cannot pass the
                    // result of an access chain to a pointer-typed parameter.
                    //
                    // Note that despite restriction rule (2) above, we still allow passing
                    // an access chain to a local variable to a function in the Slang IR, and we
                    // will introduce an additional copy when we emit SPIR-V. We do this to minimize
                    // inlining in the Slang IR, and introducing additional copy for things already
                    // in Function address space is generally less harmful.
                    return true;
                }
                return false;
            }
        case kIROp_StringType:
        case kIROp_NativeStringType:
            {
                return true;
            }
        default:
            break;
        }

        return false;
    }

    bool shouldInline(CallSiteInfo const& info)
    {
        auto callee = info.callee;

        if (doesTypeRequireInline(callee->getResultType(), nullptr, callee))
        {
            return true;
        }

        const auto count = Count(callee->getParamCount());
        for (Index i = 0; i < count; ++i)
        {
            if (doesTypeRequireInline(callee->getParamType(UInt(i)), info.call->getArg(i), callee))
            {
                return true;
            }
        }

        return false;
    }
};

} // namespace

Result performTypeInlining(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    TypeInliningPass pass(module, targetProgram);
    pass.considerAllCallSites();
    return SLANG_OK;
}

struct ForceInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    ForceInliningPass(IRModule* module)
        : Super(module)
    {
    }

    bool shouldInline(CallSiteInfo const& info)
    {
        if (info.callee->findDecoration<IRForceInlineDecoration>() ||
            info.callee->findDecoration<IRUnsafeForceInlineEarlyDecoration>() ||
            info.callee->findDecoration<IRIntrinsicOpDecoration>())
            return true;
        return false;
    }
};

void performForceInlining(IRModule* module)
{
    SLANG_PROFILE;

    ForceInliningPass pass(module);
    pass.considerAllCallSites();
}

bool performForceInlining(IRGlobalValueWithCode* func)
{
    ForceInliningPass pass(func->getModule());
    return pass.considerAllCallSitesRec(func);
}

struct PreAutoDiffForceInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    PreAutoDiffForceInliningPass(IRModule* module)
        : Super(module)
    {
    }

    Dictionary<IRInst*, bool> m_funcCanInline;

    bool shouldInline(CallSiteInfo const& info)
    {
        if (info.callee->findDecoration<IRUnsafeForceInlineEarlyDecoration>() ||
            info.callee->findDecoration<IRIntrinsicOpDecoration>())
            return true;
        bool hasForceInline = false;
        bool hasUserDefinedDerivative = false;
        for (auto decor : info.callee->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_UnsafeForceInlineEarlyDecoration:
            case kIROp_IntrinsicOpDecoration:
                return true;
            case kIROp_ForceInlineDecoration:
                hasForceInline = true;
                break;
            case kIROp_UserDefinedBackwardDerivativeDecoration:
            case kIROp_ForwardDerivativeDecoration:
                hasUserDefinedDerivative = true;
                break;
            }
        }
        if (!hasForceInline || hasUserDefinedDerivative)
        {
            return false;
        }
        if (auto result = m_funcCanInline.tryGetValue(info.callee))
            return *result;
        bool canInline = true;
        for (auto block : info.callee->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                switch (inst->getOp())
                {
                // Avoid inlining functions that have derivative instructions.
                case kIROp_ForwardDifferentiate:
                case kIROp_BackwardDifferentiate:
                case kIROp_BackwardDifferentiatePrimal:
                case kIROp_BackwardDifferentiatePropagate:
                    canInline = false;
                    goto end;

                // Also avoid inlining functions with inline-asm instructions.
                case kIROp_SPIRVAsm:
                    canInline = false;
                    goto end;
                }
            }
        }
    end:;
        m_funcCanInline[info.callee] = canInline;
        return canInline;
    }
};

bool performPreAutoDiffForceInlining(IRGlobalValueWithCode* func)
{
    PreAutoDiffForceInliningPass pass(func->getModule());
    return pass.considerAllCallSitesRec(func);
}

bool performPreAutoDiffForceInlining(IRModule* module)
{
    PreAutoDiffForceInliningPass pass(module);
    return pass.considerAllCallSitesRec(module->getModuleInst());
}

// Defined in slang-ir-specialize-resource.cpp
bool isResourceType(IRType* type);
bool isIllegalGLSLParameterType(IRType* type);

/// An inlining pass that inlines calls functions that returns resources.
/// This is needed for glsl targets.
struct GLSLResourceReturnFunctionInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    GLSLResourceReturnFunctionInliningPass(IRModule* module)
        : Super(module)
    {
    }

    bool shouldInline(CallSiteInfo const& info)
    {
        if (isResourceType(info.callee->getResultType()))
        {
            return true;
        }
        for (auto param : info.callee->getParams())
        {
            if (isIllegalGLSLParameterType(param->getDataType()))
                return true;
            auto outType = as<IROutParamTypeBase>(param->getDataType());
            if (!outType)
                continue;
            auto outValueType = outType->getValueType();
            if (isResourceType(outValueType))
                return true;
        }
        return false;
    }
};

void performGLSLResourceReturnFunctionInlining(TargetProgram* targetProgram, IRModule* module)
{
    GLSLResourceReturnFunctionInliningPass pass(module);
    bool changed = true;

    while (changed)
    {
        changed = pass.considerAllCallSites();
        simplifyIR(nullptr, module, IRSimplificationOptions::getFast(targetProgram));
    }
}

struct IntrinsicFunctionInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    IntrinsicFunctionInliningPass(IRModule* module)
        : Super(module)
    {
    }

    bool shouldInline(CallSiteInfo const& info)
    {
        auto func = as<IRFunc>(getResolvedInstForDecorations(info.callee));
        if (!func)
            return false;
        auto returnInst = as<IRReturn>(func->getFirstBlock()->getTerminator());
        if (!returnInst)
            return false;

        // If a function body has only asm blocks + trivial insts (load/store),
        // this is considered as a pure asm function, and we can inline it.
        bool hasSpvAsm = false;
        for (auto inst = func->getFirstBlock()->getFirstOrdinaryInst(); inst != returnInst;
             inst = inst->getNextInst())
        {
            switch (inst->getOp())
            {
            case kIROp_SPIRVAsmOperandInst:
            case kIROp_SPIRVAsm:
                hasSpvAsm = true;
                continue;
            case kIROp_Load:
            case kIROp_Swizzle:
            case kIROp_Store:
                continue;
            default:
                return false;
            }
        }
        return hasSpvAsm;
    }
};

void performIntrinsicFunctionInlining(IRModule* module)
{
    IntrinsicFunctionInliningPass pass(module);
    bool changed = true;

    while (changed)
    {
        changed = pass.considerAllCallSites();
    }
}

struct CustomInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    CustomInliningPass(IRModule* module)
        : Super(module)
    {
    }

    bool shouldInline(CallSiteInfo const&) { return true; }
};

bool inlineCall(IRCall* call)
{
    CustomInliningPass pass(call->getModule());
    return pass.considerCallSite(call);
}

} // namespace Slang
