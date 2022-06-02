// slang-ir-inline.cpp
#include "slang-ir-inline.h"

#include "slang-ir-ssa-simplification.h"

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

#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{
    /// Base type for inlining passes, providing shared/common functionality
struct InliningPassBase
{
        /// The module that we are optimizing/transforming
    IRModule* m_module = nullptr;

        /// Initialize an inlining pass to operate on the given `module`
    InliningPassBase(IRModule* module)
        : m_module(module)
    {
    }

        /// Consider all the call sites in the module for inliing
    bool considerAllCallSites()
    {
        return considerAllCallSitesRec(m_module->getModuleInst());
    }

        /// Consider all call sites at or under `inst` for inlining
    bool considerAllCallSitesRec(IRInst* inst)
    {
        bool changed = false;
        if( auto call = as<IRCall>(inst) )
        {
            changed = considerCallSite(call);
        }

        // Note: we defensively iterate through the child instructions
        // so that even if `child` gets removed (because of inlining)
        // we automatically start at the next instruction after it.
        //
        IRInst* next = nullptr;
        for( auto child = inst->getFirstChild(); child; child = next )
        {
            next = child->getNextInst();
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
            /// For an inlinable call, this must be non-null and a valid function *definition* (with a body) for inlining to proceed.
        IRFunc* callee = nullptr;

            /// The specialization of the function, if any.
            ///
            /// For an inlineable call, this must be non-null if the function is generic, but may be null otherwise.
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
        if(!canInline(call, callSite))
            return false;

        // If we've decided that we *can* inline the given call
        // site, we next need to check if we *should*. The rules
        // for when we should inline may vary by subclass,
        // so `shouldInline` is a virtual method.
        //
        if(!shouldInline(callSite))
            return false;

        // Finally, if we both *can* and *should* inline the
        // given call site, we hand off the a worker routine
        // that does the meat of the work.
        //
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

        /// Determine whether `call` can be inlined, and if so write information about it to `outCallSite`
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
        if( auto specialize = as<IRSpecialize>(callee) )
        {
            // If the `specialize` is applied to something other
            // than a `generic` instruction, then we can't
            // inline the call site. This can happen for a
            // call to a generic method in an interface.
            //
            IRGeneric* generic = findSpecializedGeneric(specialize);
            if(!generic)
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
            if(!callee)
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
        if(!calleeFunc)
            return false;
        //
        // If the callee *is* a function, then we can update
        // the `CalleSiteInfo` with what we've found.
        //
        outCallSite.callee = calleeFunc;

        // At this point the `CallSiteInfo` is complete and
        // could be used for inlining, but we have additional
        // checks to make.
        //
        // In particular, we should only go about inlining
        // a call site if the callee function is a full definition
        // in the IR (not just a declaration).
        //
        if(!isDefinition(calleeFunc))
            return false;

        return true;
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
        SharedIRBuilder sharedBuilder(m_module);
        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(call);

        // If the callee is a generic function, then we will
        // need to include the substitution of generic parameters
        // with their argument values in our cloning.
        //
        if( auto specialize = callSite.specialize )
        {
            auto generic = callSite.generic;

            // We start by establishing a mapping from the
            // generic parameters to the matching arguments.
            //
            Int argCounter = 0;
            for( auto param : generic->getParams() )
            {
                SLANG_ASSERT(argCounter < (Int)specialize->getArgCount());
                auto arg = specialize->getArg(argCounter++);

                env.mapOldValToNew.Add(param, arg);
            }
            SLANG_ASSERT(argCounter == (Int)specialize->getArgCount());

            // We also need to clone any instructions in the
            // body of the `generic` being specialized, since
            // these might construct types or constants that
            // reference the generic parameters.
            //
            auto body = generic->getFirstBlock();
            SLANG_ASSERT(!body->getNextBlock()); // All IR generics should have a single block.

            for( auto inst : body->getChildren() )
            {
                if( inst == callee )
                {
                    // We don't want to create a clone of the callee
                    // function at the call site, since it would
                    // immediately become dead code when we inline
                    // its body.
                }
                else if(as<IRReturn>(inst))
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
            for(auto param : callee->getParams())
            {
                SLANG_ASSERT(argCounter < (Int)call->getArgCount());
                auto arg = call->getArg(argCounter++);
                env.mapOldValToNew.Add(param, arg);
            }
            SLANG_ASSERT(argCounter == (Int)call->getArgCount());
        }

        // For now, our inlining pass only handles the case where
        // the callee is a "single-return" function, which means the callee
        // function contains only one return at the end of the body.
        if (isSingleReturnFunc(callee))
        {
            inlineSingleReturnFuncBody(callSite, &env, &builder);    
        }
        else
        {
            // Running into any non-trivial function to be inlined
            // is currently an internal compiler error.
            //
            SLANG_UNIMPLEMENTED_X("general case of inlining");
        }
    }

        /// Check if `func` represents a simple callee that has only a single `return`.
    bool isSingleReturnFunc(IRFunc* func)
    {
        auto firstBlock = func->getFirstBlock();

        // If the body block is decorated (for some reason), then the function is non-trivial.
        //
        if( firstBlock->getFirstDecoration() )
            return false;

        // If the body has more than one returns, we cannot inline it now.
        bool returnFound = false;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (inst->getOp() == kIROp_Return)
                {
                    // If the return is not at the end of the block, we cannot handle it.
                    if (inst != block->getTerminator())
                        return false;
                    // If there is already a return found, this function cannot be simple.
                    if (returnFound)
                        return false;
                    returnFound = true;
                }
            }
        }
        return true;
    }

        // When instructions are cloned, with cloneInst no sourceLoc information is copied over by default.
        // Here we attempt some policy about copying sourceLocs when inlining.
        //
        // An assumption here is that [__unsafeForceInlineEarly] will not be in user code (when we have more
        // general inlining this will not follow).
        //
        // Therefore we probably *don't* want to copy sourceLoc from the original definition in the stdlib because
        //
        // * That won't be much use to the user (they can't easily see stdlib code currently for example)
        // * That the definitions in stdlib are currently 'mundane' and largely exist to flesh out language features - such that
        //   their being in the stdlib would likely be surprising to users
        //
        // That being the case, we actually copy the call sites sourceLoc if it's defined, and only fall back
        // onto the originating loc, if that's not defined.
        //
        // We *could* vary behavior if we knew if the function was defined in the stdlib. There doesn't appear 
        // to be a decoration for this.
        // We could find out by looking at the source loc and checking if it's in the range of stdlib - this would actually be
        // a fast and easy but to do properly this way you'd want a way to mark that source range that would also work across
        // serialization.
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
    static IRInst* _cloneInstWithSourceLoc(CallSiteInfo const& callSite,
        IRCloneEnv*     env,
        IRBuilder*      builder,
        IRInst*         inst)
    {
        IRInst* clonedInst = cloneInst(env, builder, inst);
        _setSourceLoc(clonedInst, inst, callSite);
        return clonedInst;
    }

        /// Inline the body of the callee for `callSite`, where the callee has a single return.
    void inlineSingleReturnFuncBody(
        CallSiteInfo const& callSite, IRCloneEnv* env, IRBuilder* builder)
    {
        auto call = callSite.call;
        auto callee = callSite.callee;

        // We know that the callee has a single return block, so if we encounter
        // a `returnVal` instruction then it must be the one and only
        // return point for the function, and its operand will be the value
        // the callee returns.
        //
        IRInst* returnedValue = nullptr;

        // Break the basic block containing the call inst into two basic blocks.
        auto callerBlock = callSite.call->getParent();
        builder->setInsertInto(callerBlock->getParent());
        auto afterBlock = builder->createBlock();
        
        // Many operations (e.g. `cloneInst`) has define-before-use assumptions on the IR.
        // It is important to make sure we keep the ordering of blocks by inserting the
        // second half of the basic block right after `callerBlock`.
        afterBlock->insertAfter(callerBlock);
        afterBlock->sourceLoc = callSite.call->getNextInst()->sourceLoc;
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
        auto newBranch = builder->emitBranch(as<IRBlock>(env->mapOldValToNew[callee->getFirstBlock()].GetValue()));
        _setSourceLoc(newBranch, call, callSite);
        // Clone all basic blocks over to the call site.
        bool isFirstBlock = true;
        for (auto calleeBlock : callee->getBlocks())
        {
            auto clonedBlock = env->mapOldValToNew[calleeBlock].GetValue();
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
                        auto returnBranch = builder->emitBranch(afterBlock);
                        _setSourceLoc(returnBranch, inst, callSite);
                        returnedValue = findCloneForOperand(env, inst->getOperand(0));
                    }
                    break;
                }
            }
            isFirstBlock = false;
        }

        // If there was a `returnVal` instruction that established
        // the return value of the inlined function, then that value
        // should be used to replace any uses of the original call.
        //
        if( returnedValue )
        {
            call->replaceUsesWith(returnedValue);
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
    {}

    bool shouldInline(CallSiteInfo const& info)
    {
        if(info.callee->findDecoration<IRUnsafeForceInlineEarlyDecoration>())
            return true;
        return false;
    }
};


void performMandatoryEarlyInlining(IRModule* module)
{
    MandatoryEarlyInliningPass pass(module);
    pass.considerAllCallSites();
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
    {}

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

void performGLSLResourceReturnFunctionInlining(IRModule* module)
{
    GLSLResourceReturnFunctionInliningPass pass(module);
    bool changed = true;

    while (changed)
    {
        changed = pass.considerAllCallSites();
        simplifyIR(module);
    }
}

struct CustomInliningPass : InliningPassBase
{
    typedef InliningPassBase Super;

    CustomInliningPass(IRModule* module)
        : Super(module)
    {}

    bool shouldInline(CallSiteInfo const&)
    {
        return true;
    }
};

bool inlineCall(IRCall* call)
{
    CustomInliningPass pass(call->getModule());
    return pass.considerCallSite(call);
}


} // namespace Slang
