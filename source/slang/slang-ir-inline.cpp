// slang-ir-inline.cpp
#include "slang-ir-inline.h"

#include "../core/slang-io.h"
#include "../core/slang-performance-profiler.h"
#include "../core/slang-std-writers.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"

#include <stack>
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

    /// Find an existing debug function for the given function
    IRInst* findExistingDebugFunc(IRFunc* func)
    {
        if (!func)
            return nullptr;

        // Look for a DebugFunctionDecoration on the function
        for (auto decor : func->getDecorations())
        {
            if (decor->getOp() == kIROp_DebugFunctionDecoration)
            {
                return decor->getOperand(0);
            }
        }
        return nullptr;
    }

    /// Setup debug information for the inlined call site
    IRInst* setupDebugInfoForInlinedCall(
        IRCall* call,
        IRFunc* callee,
        IRBuilder& builder,
        IRInst** outDebugFunc)
    {
        IRDebugLine* lastDebugLine = nullptr;
        IRInst* debugInlinedAt = nullptr;
        IRInst* debugFuncParent = nullptr;
        IRInst* debugFuncCurrent = nullptr;

        if (!callee->findDecoration<IRDebugLocationDecoration>())
        {
            return nullptr;
        }
        else
        {
            // Set insertion point to module's global scope. Insert it right after
            // the DebugSource instruction.
            IRInst* insertPoint = m_module->getModuleInst()->getFirstChild();
            while (insertPoint)
            {
                if (as<IRDebugSource>(insertPoint))
                {
                    break;
                }
                insertPoint = insertPoint->getNextInst();
            }
            builder.setInsertAfter(insertPoint);

            for (IRInst* inst = call->getPrevInst(); inst; inst = inst->getPrevInst())
            {
                if (auto debugLine = as<IRDebugLine>(inst))
                {
                    lastDebugLine = debugLine;
                    break;
                }
            }

            if (lastDebugLine)
            {
                auto parentFunc = getParentFunc(call);

                // Find or create debug function for current function
                debugFuncCurrent = findExistingDebugFunc(callee);
                if (!debugFuncCurrent)
                {
                    auto nameHint = callee->findDecoration<IRNameHintDecoration>();
                    IRStringLit* nameOperand = nameHint ? as<IRStringLit>(nameHint->getNameOperand()) : nullptr;
                    if (nameOperand)
                    {
                        auto locationDecor = callee->findDecoration<IRDebugLocationDecoration>();
                        IRInst* debugType = callee->getDataType();

                        debugFuncCurrent = builder.emitDebugFunction(
                            nameOperand,
                            locationDecor->getLine(),
                            locationDecor->getCol(),
                            locationDecor->getSource(),
                            debugType);
                        
                        // Add a decoration to link the function to its debug function
                        builder.addDecoration(callee, kIROp_DebugFunctionDecoration, debugFuncCurrent);
                    }
                }

                // Find or create debug function for parent function
                debugFuncParent = findExistingDebugFunc(parentFunc);
                if (!debugFuncParent && parentFunc)
                {
                    auto nameHint = parentFunc->findDecoration<IRNameHintDecoration>();
                    IRStringLit* nameOperand = nameHint ? as<IRStringLit>(nameHint->getNameOperand()) : nullptr;
                    if (nameOperand)
                    {
                        IRInst* debugType = parentFunc->getDataType();

                        auto locationDecor = parentFunc->findDecoration<IRDebugLocationDecoration>();
                        if (!locationDecor)
                            locationDecor = callee->findDecoration<IRDebugLocationDecoration>();

                        debugFuncParent = builder.emitDebugFunction(
                            nameOperand,
                            locationDecor->getLine(),
                            locationDecor->getCol(),
                            locationDecor->getSource(),
                            debugType);
                        
                        // Add a decoration to link the function to its debug function
                        builder.addDecoration(parentFunc, kIROp_DebugFunctionDecoration, debugFuncParent);
                    }
                }

                // The debugInlinedAt needs to be cloned for each inlining call because our
                // algorithm updates the operand. So we need to reset the insert point here.
                builder.setInsertBefore(call);
                debugInlinedAt = builder.emitDebugInlinedAt(
                    lastDebugLine->getLineStart(),
                    lastDebugLine->getColStart(),
                    lastDebugLine->getSource(),
                    nullptr,
                    debugFuncParent);

                if (outDebugFunc)
                    *outDebugFunc = debugFuncCurrent;
            }

            if (debugInlinedAt)
            {
                return debugInlinedAt;
            }
        }
        return nullptr;
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
            IRDebugLine* lastDebugLine = nullptr;
            for (IRInst* inst = call->getPrevInst(); inst; inst = inst->getPrevInst())
            {
                if (auto debugLine = as<IRDebugLine>(inst))
                {
                    lastDebugLine = debugLine;
                    break;
                }
            }

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

                if (lastDebugLine)
                {
                    builder.emitDebugInlinedAt(
                        lastDebugLine->getLineStart(),
                        lastDebugLine->getColStart(),
                        lastDebugLine->getSource(),
                        nullptr,
                        newCall);
                }
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

        IRInst* debugFunc = nullptr;
        auto debugInlinedAt = setupDebugInfoForInlinedCall(call, callee, builder, &debugFunc);

        inlineFuncBody(callSite, &env, &builder, debugInlinedAt, debugFunc);
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
        IRInst* debugInlinedAt,
        IRInst* debugFunc)
    {
        auto call = callSite.call;
        auto callee = callSite.callee;

        // The callee had better have only a single basic block.
        //
        auto firstBlock = callee->getFirstBlock();
        SLANG_ASSERT(!firstBlock->getNextBlock());

        // emit debug information
        if (callee->findDecoration<IRDebugLocationDecoration>() && debugInlinedAt)
        {
            IRInst* firstOrdinaryInst = callee->getFirstBlock()->getFirstOrdinaryInst();
            IRInst* lastOrdinaryInst = callee->getFirstBlock()->getLastOrdinaryInst();
            bool newScopeEmitted = false;

            auto debugScope = as<IRDebugScope>(firstOrdinaryInst);
            if (!debugScope)
            {
                setInsertBeforeOrdinaryInst(builder, firstOrdinaryInst);
                builder->emitDebugScope(debugFunc, debugInlinedAt);
                newScopeEmitted = true;
            }
            else
            {
                debugScope->setInlinedAt(debugInlinedAt);
            }
            // Check if the last ordinary instruction is a terminator
            if (as<IRTerminatorInst>(lastOrdinaryInst))
            {
                setInsertBeforeOrdinaryInst(builder, lastOrdinaryInst);
            }
            else
            {
                setInsertAfterOrdinaryInst(builder, lastOrdinaryInst);
            }
            if (newScopeEmitted)
            {
                builder->emitDebugNoScope();
            }
        }

        // We will loop over the instructions in the block and clone
        // them into the same basic block as the `call`.
        //
        builder->setInsertBefore(call);

        // Along the way, we will detect any `return` instruction,
        // and remember the (clone of the) returned value.
        //
        IRInst* returnVal = nullptr;

        for (auto inst : firstBlock->getChildren())
        {
            switch (inst->getOp())
            {
            default:
                // In the common case we just clone the instruction as-is
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
    void inlineFuncBody(
        CallSiteInfo const& callSite,
        IRCloneEnv* env,
        IRBuilder* builder,
        IRInst* debugInlinedAt,
        IRInst* debugFunc)
    {
        auto callee = callSite.callee;

        // If the callee consists of a single basic block *and* that block
        // ends with a `return` instruction, then we can apply a simple approach
        // to inlining that is compatible with any call site (including those
        // at the global scope).
        //
        auto firstBlock = callee->getFirstBlock();
        SLANG_ASSERT(firstBlock);
        if (!firstBlock->getNextBlock() && as<IRReturn>(firstBlock->getTerminator()))
        {
            inlineSingleBlockFuncBody(callSite, env, builder, debugInlinedAt, debugFunc);
            return;
        }

        // If the callee has multiple blocks, use the more complex inlining approach
        inlineMultipleBlockFuncBody(callSite, env, builder, debugInlinedAt, debugFunc);
    }

    /// Inline the body of the callee for `callSite`, for a callee that has multiple basic blocks.
    void inlineMultipleBlockFuncBody(
        CallSiteInfo const& callSite,
        IRCloneEnv* env,
        IRBuilder* builder,
        IRInst* debugInlinedAt,
        IRInst* debugFunc)
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

        // Insert debug info in the callee blocks so they get cloned as well.
        if (debugInlinedAt && callee->findDecoration<IRDebugLocationDecoration>())
        {
            insertDebugScopeForMultiBlock(callee, builder, debugFunc, debugInlinedAt);
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

        // Update debug info in the cloned blocks
        if (debugInlinedAt && callee->findDecoration<IRDebugLocationDecoration>())
        {
            updateDebugInlinedAt(callee, env, debugInlinedAt);
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

    /// Insert debug scope information in the callee blocks before cloning
    ///
    /// This function sets up debug scope information in each callee block before they're cloned.
    /// For each block, we ensure:
    /// 1. A DebugScope instruction is placed at the beginning of the block
    /// 2. A DebugNoScope instruction is placed at the end of the block
    ///
    /// This creates a proper nested debug scope structure that ensures debug information
    /// is correctly preserved during the inlining process.
    void insertDebugScopeForMultiBlock(
        IRFunc* callee,
        IRBuilder* builder,
        IRInst* debugFunc,
        IRInst* debugInlinedAt)
    {
        for (auto calleeBlock : callee->getBlocks())
        {
            IRInst* firstOrdinaryInst = calleeBlock->getFirstOrdinaryInst();
            IRInst* lastOrdinaryInst = calleeBlock->getLastOrdinaryInst();
            bool newScopeEmitted = false;

            // If we had cloned a single block function, after cloning into the calleeblock,
            // the DebugScope may no longer be the FirstOrdinaryInst in the callee's block.
            // So we need to find out the first DebugScope function by iteration.
            IRInst* firstDebugScope = nullptr;
            for (auto inst : calleeBlock->getChildren())
            {
                if (as<IRDebugScope>(inst))
                {
                    firstDebugScope = inst;
                    break;
                }
            }
            // Now handle different scenarios.
            // 1. We have a block that's belonging to the current inlined function.
            if (firstDebugScope == nullptr)
            {
                setInsertBeforeOrdinaryInst(builder, firstOrdinaryInst);
                builder->emitDebugScope(debugFunc, debugInlinedAt);
                newScopeEmitted = true;
            }
            // 2. In this block we find a DebugScope. This is a debugscope of another
            //    inlined function. So we have to emit a new scope for the current function.
            else if (
                firstDebugScope != nullptr &&
                as<IRDebugScope>(firstDebugScope)->getScope() != debugFunc)
            {
                setInsertBeforeOrdinaryInst(builder, firstOrdinaryInst);
                builder->emitDebugScope(debugFunc, debugInlinedAt);
                newScopeEmitted = true;
            }
            // 3. The same function is inlined in another place. So we just update the
            // debugInlinedAt.
            else
            {
                as<IRDebugScope>(firstDebugScope)->setInlinedAt(debugInlinedAt);
            }

            // Emit a debugNoScope if needed:
            {
                if (as<IRTerminatorInst>(lastOrdinaryInst))
                    setInsertBeforeOrdinaryInst(builder, lastOrdinaryInst);
                else
                    setInsertAfterOrdinaryInst(builder, lastOrdinaryInst);

                if (newScopeEmitted)
                    builder->emitDebugNoScope();
            }
        }
    }

    // Update debug inlined-at information in the cloned blocks. This is a crucial step
    // in setting up debug information. The debug information is mainly setup as follows:
    //
    //  1. Setup Debug scopes:
    //  =====================
    // In case of single block, we just set the first and last instruction of the block
    // as DebugScope/DebugNoScope.
    // In case of multiple blocks, we do: For each block:
    //      DebugScope
    //          <insts>
    //          DebugScope
    //              <insts>
    //          DebugNoScope
    //      DebugNoScope
    // Note: For each block, we always make sure that the DebugScope is the first
    // instruction. Look at insertDebugScopeForMultiBlock for details.
    //
    // 2. Setup DebugInlinedAt
    // =======================
    // We gather the inlinedAt information just before the the inlining process.
    // This is done in setupDebugInfoForInlinedCall. Now this does not have access to the
    // outerInlined there because the parent would not be inlined yet.
    //
    // 3. Update DebugInlinedAt
    // ========================
    // The next step of figuring out the outerInlined is done in the below updateDebugInlinedAt.
    // To obtain the outerInlined, the algorithm is like so:
    //  - We keep pushing into the stack until we find the last scope.
    //  - Once we find the last debug scope in the call chain, we then start updating the
    //  - InlinedAt information. So, for Call i, we need to set outer as i-1.
    void updateDebugInlinedAt(IRFunc* callee, IRCloneEnv* env, IRInst* debugInlinedAt)
    {
        std::stack<IRInst*> inlinedAtStack;

        for (auto calleeBlock : callee->getBlocks())
        {
            auto clonedBlock = env->mapOldValToNew.getValue(calleeBlock);
            if (!clonedBlock)
                continue;

            for (auto inst : clonedBlock->getChildren())
            {
                if (auto debugScope = as<IRDebugScope>(inst))
                {
                    IRInst* outerInlinedAt =
                        inlinedAtStack.empty() ? nullptr : inlinedAtStack.top();
                    IRDebugInlinedAt* inlinedAt = as<IRDebugInlinedAt>(debugScope->getInlinedAt());

                    if (outerInlinedAt && (inlinedAt != outerInlinedAt))
                    {
                        inlinedAt->setOuterInlinedAt(outerInlinedAt);
                        inlinedAtStack.push(inlinedAt);
                    }
                    else
                    {
                        inlinedAtStack.push(debugInlinedAt);
                    }
                }
                else if (as<IRDebugNoScope>(inst) && !inlinedAtStack.empty())
                {
                    inlinedAtStack.pop();
                }
            }
        }
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

    TypeInliningPass(IRModule* module)
        : Super(module)
    {
    }

    bool doesTypeRequireInline(IRType* type, IRFunc* callee)
    {
        // TODO(JS):
        // I guess there is a question here about what type around string requires
        // inlining.
        // For example if we had an array of strings etc.
        // For now we just consider just basic string types.
        const auto op = type->getOp();
        switch (op)
        {
        case kIROp_RefType:
            {
                if (callee->findDecoration<IRNoRefInlineDecoration>())
                    return false;
                return true;
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

        if (doesTypeRequireInline(callee->getResultType(), callee))
        {
            return true;
        }

        const auto count = Count(callee->getParamCount());
        for (Index i = 0; i < count; ++i)
        {
            if (doesTypeRequireInline(callee->getParamType(UInt(i)), callee))
            {
                return true;
            }
        }

        return false;
    }
};

} // namespace

Result performTypeInlining(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    // TODO(JS):
    // This is perhaps not as efficient as might be desirable.
    // A more optimized version might not need to pass over all of the module
    // to find new call sites.
    //
    // Another problem here is recursion. Right now Slang compiler doesn't accept recursive input,
    // but the Slang language is supposed to support recursion on targets that support it.
    // There are GPU targets that allow recursion such as CUDA.
    //
    // Another approach would be (when enabled) when inlining occurs, would be instead of continuing
    // *after*, to start the checks/inlining from where the inline took place.
    //
    while (true)
    {
        TypeInliningPass pass(module);
        if (pass.considerAllCallSites())
        {
            // If there was a change try inlining again
            continue;
        }

        // Done.
        break;
    }


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
            auto outType = as<IROutTypeBase>(param->getDataType());
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
            case kIROp_swizzle:
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
