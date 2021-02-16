// slang-ir-inline.cpp
#include "slang-ir-inline.h"

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
    void considerAllCallSites()
    {
        considerAllCallSitesRec(m_module->getModuleInst());
    }

        /// Consider all call sites at or under `inst` for inlining
    void considerAllCallSitesRec(IRInst* inst)
    {
        if( auto call = as<IRCall>(inst) )
        {
            considerCallSite(call);
        }

        // Note: we defensively iterate through the child instructions
        // so that even if `child` gets removed (because of inlining)
        // we automatically start at the next instruction after it.
        //
        IRInst* next = nullptr;
        for( auto child = inst->getFirstChild(); child; child = next )
        {
            next = child->getNextInst();
            considerAllCallSitesRec(child);
        }
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
    void considerCallSite(IRCall* call)
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
            return;

        // If we've decided that we *can* inline the given call
        // site, we next need to check if we *should*. The rules
        // for when we should inline may vary by subclass,
        // so `shouldInline` is a virtual method.
        //
        if(!shouldInline(callSite))
            return;

        // Finally, if we both *can* and *should* inline the
        // given call site, we hand off the a worker routine
        // that does the meat of the work.
        //
        inlineCallSite(callSite);
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
        SharedIRBuilder sharedBuilder;
        sharedBuilder.session = m_module->getSession();
        sharedBuilder.module = m_module;
        IRBuilder builder;
        builder.sharedBuilder = &sharedBuilder;
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
        // the callee is a "trivial" function, which can support
        // a very simple approach to inlining.
        //
        if( isTrivialFunc(callee) )
        {
            inlineTrivialFuncBody(callSite, &env, &builder);
        }
        else
        {
            // Running into any non-trivial function to be inlined
            // is currently an internal compiler error.
            //
            SLANG_UNIMPLEMENTED_X("general case of inlining");
        }
    }

        /// Check if `func` represents a trivial single-block callee that can be inlined simply
    bool isTrivialFunc(IRFunc* func)
    {
        // The function must have a single bocy block to be trivial.
        //
        auto firstBlock = func->getFirstBlock();
        if( firstBlock->getNextBlock() )
            return false;

        // If the body block is decorated (for some reason), then the function is non-trivial.
        //
        if( firstBlock->getFirstDecoration() )
            return false;

        // If the body block terminates in something other than a `return` then the function is non-trivial.
        //
        auto terminator = firstBlock->getTerminator();
        if( !as<IRReturn>(terminator) )
            return false;

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
    static IRInst* _cloneInstWithSourceLoc(CallSiteInfo const& callSite,
        IRCloneEnv*     env,
        IRBuilder*      builder,
        IRInst*         inst)
    {
        IRInst* clonedInst = cloneInst(env, builder, inst);

        SourceLoc sourceLoc;

        if (callSite.call->sourceLoc.isValid())
        {
            // Default to using the source loc at the call site
            sourceLoc = callSite.call->sourceLoc;
        }
        else if (inst->sourceLoc.isValid())
        {
            // If we don't have that copy the inst being cloned sourceLoc
            sourceLoc = inst->sourceLoc;
        }

        clonedInst->sourceLoc = sourceLoc;
        return clonedInst;
    }

        /// Inline the body of the callee for `callSite`, where the callee is trivial as tested by `isTrivialFunc`
    void inlineTrivialFuncBody(CallSiteInfo const& callSite, IRCloneEnv* env, IRBuilder* builder)
    {
        auto call = callSite.call;
        auto callee = callSite.callee;
        auto firstBlock = callee->getFirstBlock();

        // We know that the callee has a single block, so if we encounter
        // a `returnVal` instruction then it must be the one and only
        // return point for the block, and its operand will be the value
        // the calee returns.
        //
        IRInst* returnedValue = nullptr;

        // We will loop over the instructions of the one and only block,
        // and clone each of them appropriately.
        //
        for( auto inst : firstBlock->getChildren() )
        {
            switch( inst->getOp() )
            {
            default:
                // The default value is to clone the instruction using
                // the existing cloning infrastructure and the `env`
                // we have already set up.
                //
                // SourceLoc information is copied if there is appropriate data available.
                _cloneInstWithSourceLoc(callSite, env, builder, inst);
                break;

            case kIROp_Param:
                // Parameters can be completely ignored in the single-block
                // case, because they have all been replaced via `env`.
                break;

            case kIROp_ReturnVoid:
                // A return with no operand can be ignored, since a return
                // from the inlined call should just continue after the
                // call site.
                //
                break;

            case kIROp_ReturnVal:
                // A return with a value is similar to `returnVoid` except
                // that we need to note the (clone of the) value being
                // returned, so that we can use it to replace the value
                // of the original call.
                //
                returnedValue = findCloneForOperand(env, inst->getOperand(0));
                break;
            }
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

} // namespace Slang
