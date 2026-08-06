#include "slang-ir-transform-params-to-constref.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-target.h"

namespace Slang
{

struct TransformParamsToConstRefContext
{
    IRModule* module;
    DiagnosticSink* sink;
    IRBuilder builder;
    bool changed = false;

    // When set (CUDA only), a call argument that is an entry-point by-value uniform aggregate
    // parameter is forwarded by address instead of via a temporary copy. See the header comment on
    // `transformParamsToConstRef`.
    bool forwardEntryPointUniformAddress;

    TransformParamsToConstRefContext(
        IRModule* module,
        DiagnosticSink* sink,
        bool forwardEntryPointUniformAddress)
        : module(module)
        , sink(sink)
        , builder(module)
        , forwardEntryPointUniformAddress(forwardEntryPointUniformAddress)
    {
    }

    // Check if a type should be transformed (struct, array, or other composite types)
    virtual bool shouldTransformParam(IRParam* param)
    {
        auto type = param->getDataType();
        if (!type)
            return false;

        switch (type->getOp())
        {
        case kIROp_StructType:
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_TupleType:
            // valid type, continue to check
            break;
        default:
            return false;
        }

        return true;
    }

    void rewriteValueUsesToAddrUses(IRInst* newAddrInst)
    {
        // The overall strategy here is as follows:
        //
        // - First, insert IRLoad() in front of all uses of newAddrInst. This
        //   is a value parameter turned into pointer to value. Add all IRLoad()
        //   instructions in the working set
        // - Then, for every inserted IRLoad() instruction, search for
        //   IRFieldExtract(IRLoad(ptr), ...) and IRGetElement(IRLoad(ptr), ...) patterns,
        //   and transform these to IRLoad(IRFieldAddress(ptr, ...)) and
        //   IRLoad(IRGetElementPtr(ptr, ...)), and insert the new IRLoad()
        //   instructions in the working set
        //   - Remove also stores to write-once temporary variables that are
        //     immediately passed into a constref location in a call (see below)
        //   - If all uses of the inserted IRLoad() were translated, remove the
        //     IRLoad() to keep this pass clean

        List<IRLoad*> workList;

        traverseUses(
            newAddrInst,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                builder.setInsertBefore(user);
                IRLoad* loadInst = as<IRLoad>(builder.emitLoad(newAddrInst));
                use->set(loadInst);

                workList.add(loadInst);
            });

        for (Index i = 0; i < workList.getCount(); i++)
        {
            IRLoad* loadInst = workList[i];
            bool allUsesTranslated = true;

            traverseUses(
                loadInst,
                [&](IRUse* use)
                {
                    IRInst* userInst = use->getUser();
                    bool useTranslated = false;

                    switch (userInst->getOp())
                    {
                    case kIROp_FieldExtract:
                        {
                            if (isUseBaseAddrOperand(use, userInst))
                            {
                                // Transform IRFieldExtract(IRLoad(ptr), x)
                                //           ==>
                                //           IRLoad(IRFieldAddr(ptr), x)

                                auto fieldExtract = as<IRFieldExtract>(userInst);
                                builder.setInsertBefore(fieldExtract);
                                auto fieldAddr = builder.emitFieldAddress(
                                    builder.getPtrType(userInst->getDataType()),
                                    loadInst->getPtr(),
                                    fieldExtract->getField());
                                auto loadFieldAddr = as<IRLoad>(builder.emitLoad(fieldAddr));
                                fieldExtract->replaceUsesWith(loadFieldAddr);
                                fieldExtract->removeAndDeallocate();

                                workList.add(loadFieldAddr);
                                useTranslated = true;
                            }
                            break;
                        }

                    case kIROp_GetElement:
                        {
                            if (isUseBaseAddrOperand(use, userInst))
                            {
                                // Transform IRGetElement(IRLoad(ptr), x)
                                //           ==>
                                //           IRLoad(IRGetElementPtr(ptr), x)

                                auto getElement = as<IRGetElement>(userInst);
                                builder.setInsertBefore(getElement);
                                auto getElementPtr = builder.emitElementAddress(
                                    builder.getPtrType(userInst->getDataType()),
                                    loadInst->getPtr(),
                                    getElement->getIndex());
                                auto loadElementPtr = as<IRLoad>(builder.emitLoad(getElementPtr));
                                getElement->replaceUsesWith(loadElementPtr);
                                getElement->removeAndDeallocate();

                                workList.add(loadElementPtr);
                                useTranslated = true;
                            }
                            break;
                        }

                    case kIROp_Store:
                        {
                            // If the current value is being stored into a write-once temp var that
                            // is immediately passed into a constref location in a call, we can get
                            // rid of the temp var and replace it with `inst` directly.
                            // (such temp var can be introduced during `updateCallSites` when we
                            // were processing the callee.)

                            // Transform IRStore(storeDest, load(ptr)) where storeDest has attribute
                            //                                     TempCallArgImmutableVarDecoration
                            //           IRInst(storeDest)
                            //           ==>
                            //           IRInst(ptr)

                            auto storeInst = as<IRStore>(userInst);
                            auto storeDest = storeInst->getPtr();

                            if (storeInst->getValUse() == use &&
                                storeDest->findDecorationImpl(
                                    kIROp_TempCallArgImmutableVarDecoration))
                            {
                                storeDest->replaceUsesWith(loadInst->getPtr());
                                userInst->removeAndDeallocate();
                                storeDest->removeAndDeallocate();
                                useTranslated = true;
                            }
                            break;
                        }
                    }

                    allUsesTranslated = allUsesTranslated && useTranslated;
                });

            if (allUsesTranslated)
                loadInst->removeAndDeallocate();
        }
    }

    void rewriteParamUseSitesToSupportConstRefUsage(HashSet<IRParam*>& updatedParams)
    {
        // Traverse the uses of our updated params to rewrite them.
        // Assume a `in` parameter has been converted to a `constref` parameter.
        for (auto param : updatedParams)
        {
            rewriteValueUsesToAddrUses(param);
        }
    }

    // Check if `load` is an `IRLoad(addr)` where `addr` is a immutable location.
    IRInst* isLoadFromImmutableAddress(IRInst* load)
    {
        if (load->getOp() != kIROp_Load)
            return nullptr;
        auto addr = load->getOperand(0);
        auto root = getRootAddr(addr);
        if (!root)
            return nullptr;
        if (!root->getDataType())
            return nullptr;
        switch (root->getDataType()->getOp())
        {
        case kIROp_ConstantBufferType:
        case kIROp_BorrowInParamType:
        case kIROp_ParameterBlockType:
            return addr;
        default:
            // Note that we should in general not assume a read-only StructuredBuffer or
            // a pointer with read-only access as an immutable location due to potential aliasing.
            // We could introduce a compiler flag to turn on optimizations on these buffer types
            // assuming there is no aliasing.
            break;
        }
        return nullptr;
    }

    // True if the address of `arg` can be forwarded into the `borrow in` callee at a call site,
    // instead of copying the whole aggregate into a per-thread temporary (the #11774 slowdown).
    //
    // Retyping the parameter to a pointer is all-or-nothing across it, so this only holds when
    // every call use lands in a callee slot this pass also rewrites to `borrow in` - otherwise that
    // callee would be handed a pointer for a by-value `T`. An already-forwarded parameter (the
    // multi-forward case) trivially qualifies; it is the pointer to forward again.
    //
    // Returns the validated `IRParam` on success so the caller can forward it without repeating the
    // cast, and null when `arg` is not forwardable - so `forwardParamStorageAddress`'s non-null
    // precondition is carried by the return value rather than left to a duplicated `as<IRParam>`.
    IRParam* findForwardableParamStorage(IRInst* arg)
    {
        auto param = as<IRParam>(arg);
        if (!param)
            return nullptr;
        // Already forwarded, so the call-use loop below has already run for it and passed:
        // `forwardParamStorageAddress` is the only place that ever sets this address space, and it
        // only runs on a parameter this function just approved. Re-validating would be redundant -
        // but the shortcut does assume no *new* callee has entered the picture since. That holds
        // because `updateCallSites` only rebuilds calls that already existed, one per original
        // call, each keeping its callee; it never routes the parameter to an unseen callee.
        // A future change that forwarded a parameter into a genuinely new call would have to re-run
        // the loop rather than land here.
        if (isCudaKernelParamBorrowInType(param->getDataType()))
            return param;
        if (!isEntryPointByValueUniformAggregateParam(param))
            return nullptr;

        for (auto use = param->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            // Non-call uses need no check here: `param` is by-value, so as an SSA value it cannot
            // be a store destination or have its address taken, leaving plain value reads that
            // `forwardParamStorageAddress` redirects through the one hoisted load.
            if (!call)
                continue;
            // Checking the callee rather than the slot it lands in is deliberate. `param` is a
            // struct/sized-array aggregate and `shouldTransformParam` rewrites every such slot
            // unconditionally, so a transformable callee necessarily rewrites this one. Querying
            // the slot directly would not be equivalent: functions are processed in arbitrary
            // order, and `shouldTransformParam` is a pre-transform predicate that returns false
            // once a slot has been retyped to `borrow in`, so an already-processed callee would
            // read as untransformable.
            //
            // Both decline reasons keep a by-value callee from being handed a `Big_0 *`, and each
            // has its own test because they are reached differently.
            // `cuda-forward-uniform-signature-preserved-callee.slang` covers a callee also used as
            // a function value; `cuda-forward-uniform-unprocessed-callee.slang` covers one the pass
            // refuses to rewrite at all (`[CudaDeviceExport]`, externally visible signature).
            auto callee = as<IRFunc>(call->getCallee());
            if (!callee || !shouldProcessFunction(callee) || hasSignaturePreservingUse(callee))
                return nullptr;
        }
        return param;
    }

    // Forward the address of an entry-point uniform aggregate parameter into a `borrow in` callee.
    // An `IRParam` is an SSA value with no storage to take `GetAddress` of, so instead the
    // parameter is retyped to `ConstRef<T, CudaKernelParam>` and *is* the pointer value to forward;
    // its in-kernel value reads are redirected through one inserted `IRLoad` so they still observe
    // a `T`. Returns `param` itself, not a freshly-built address instruction.
    //
    // Precondition: `param` came from `findForwardableParamStorage`, which guarantees no surviving
    // call use reads the parameter by value.
    IRInst* forwardParamStorageAddress(IRParam* param)
    {
        auto valueType = param->getDataType();
        SLANG_ASSERT(valueType);
        // Idempotency guard for the multi-forward case: a parameter forwarded to a second call site
        // is already the pointer, so reuse it without a second retype or load. See
        // `cuda-forward-uniform-multi-forward.slang`.
        if (isCudaKernelParamBorrowInType(valueType))
            return param;

        // Collect the value reads before changing anything. Call arguments are excluded: they
        // either forward the pointer or belong to the old calls `updateCallSites` is about to
        // discard, and the precondition guarantees none of them reads the parameter by value -
        // `findForwardableParamStorage` returned it, which means every call use of `param`
        // lands in a callee slot this pass rewrites to a `borrow in` pointer, so no surviving call
        // reads it as a `T`.
        List<IRUse*> readUses;
        for (auto use = param->firstUse; use; use = use->nextUse)
        {
            if (!as<IRCall>(use->getUser()))
                readUses.add(use);
        }

        param->setFullType(builder.getBorrowInParamType(valueType, AddressSpace::CudaKernelParam));

        // Retyping the parameter changed the entry function's signature, so rebuild the parent
        // function's type from its (now-updated) parameter types. `processFunc` only calls
        // `fixUpFuncType` for the transformed callee; the entry point is skipped there, so the fix
        // must happen here or the function type would keep saying `T` while the param says
        // `ConstRef<T, CudaKernelParam>`.
        auto entryBlock = as<IRBlock>(param->getParent());
        auto entryFunc = as<IRFunc>(entryBlock->getParent());
        SLANG_ASSERT(entryFunc);

        // One load suffices for all reads: every value read of an entry-point parameter is an
        // ordinary inst, and the entry block's first ordinary inst dominates every ordinary inst in
        // the function, so this single load dominates all the uses it replaces.
        //
        // Save and restore the shared builder's insert location: the sole caller
        // (`updateCallSites`) sets it to the original call and then rebuilds that call after this
        // returns, so leaking the entry-block insert point here would mis-place the rebuilt call at
        // the top of the entry block - before the definitions of its own arguments (use-before-def
        // / broken SSA).
        if (readUses.getCount())
        {
            auto savedLoc = builder.getInsertLoc();
            builder.setInsertBefore(entryBlock->getFirstOrdinaryInst());
            auto loaded = builder.emitLoad(valueType, param);
            builder.setInsertLoc(savedLoc);
            for (auto use : readUses)
                use->set(loaded);
        }

        fixUpFuncType(entryFunc);
        return param;
    }

    // Update call sites to pass an address instead of value for each updated-param
    void updateCallSites(IRFunc* func, HashSet<IRParam*>& updatedParams)
    {
        // Find all calls which use `func`.
        List<IRCall*> callsToUpdate;
        traverseUsers<IRCall>(func, [&](IRCall* call) { callsToUpdate.add(call); });

        // Update each call site
        for (auto call : callsToUpdate)
        {
            builder.setInsertBefore(call);
            List<IRInst*> newArgs;

            // Transform arguments to match the updated-parameter
            UInt i = 0;
            for (IRParam* param = func->getFirstParam(); param; param = param->getNextParam(), i++)
            {
                auto arg = call->getArg(i);
                if (!updatedParams.contains(param))
                {
                    newArgs.add(arg);
                    continue;
                }
                if (auto addr = isLoadFromImmutableAddress(arg))
                {
                    // If existing argument is a load from an immutable buffer address,
                    // we can pass in the address as is, without making a temporary copy.
                    newArgs.add(addr);
                }
                else if (
                    auto forwardable = forwardEntryPointUniformAddress
                                           ? findForwardableParamStorage(arg)
                                           : nullptr)
                {
                    // Forward the aggregate by address instead of copying it into a per-thread
                    // temporary - the #11774 slowdown.
                    //
                    // The forwarded argument is `ConstRef<T, CudaKernelParam>` while the callee
                    // slot stays `ConstRef<T, Generic>` (set in `processFunc`). The address spaces
                    // must differ, because this atom is what tells the CPP emitter to print a
                    // parameter as by-value `T p` rather than `T*` (`emitSimpleTypeImpl`'s
                    // `kIROp_BorrowInParamType` case). The entry point wants that by-value spelling
                    // to keep its kernel ABI; the callee must keep `T*` so it can receive the
                    // address. Giving the callee slot `CudaKernelParam` too makes it declare
                    // `readsum_0(Big_0 b_0, ...)` while still being called as
                    // `readsum_0(&big_0, ...)` - a pointer into a by-value parameter, which NVRTC
                    // rejects. The divergence is the mechanism, not an oversight.
                    //
                    // Nothing between here and CUDA emit compares the two: `validateIRInstOperand`
                    // in `slang-ir-validate.cpp` checks operand back-pointers and that a def
                    // dominates its uses, never an argument's type against the callee's parameter
                    // type. A future call-argument type check there must permit this pairing.
                    // `cuda-forward-uniform-struct-forward.slang` pins both halves in the emitted
                    // text - the by-value `computeMain(Big_0 big_0, ...)` and the pointer-taking
                    // `readsum_0(Big_0 * b_0, ...)` it is passed to - so collapsing the two atoms
                    // into one fails that test rather than silently changing the ABI.
                    //
                    // No write can reach the forwarded storage: an `IRStore` destination must be
                    // pointer-typed and a by-value `IRParam` is an SSA value, so the front end
                    // proxies any source-level mutation onto a fresh local; the callee slot is
                    // `borrow in` and copies the pointee before any write. That is a type-system
                    // guarantee, not a convention.
                    newArgs.add(forwardParamStorageAddress(forwardable));
                }
                else
                {
                    auto tempVar = builder.emitVar(arg->getFullType());
                    builder.addDecoration(tempVar, kIROp_TempCallArgImmutableVarDecoration);
                    builder.emitStore(tempVar, arg);
                    newArgs.add(tempVar);
                }
            }

            // Create new call with updated arguments
            auto newCall = builder.emitCallInst(call->getFullType(), func, newArgs);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }
    }

    // Check if function should be excluded from transformation
    virtual bool shouldProcessFunction(IRFunc* func)
    {
        // Skip functions without definitions
        if (!func->isDefinition())
            return false;

        // Skip if we find any of these decorations
        for (auto decoration : func->getDecorations())
        {
            // Skip functions with target intrinsic decorations.
            // These functions cannot be properly legalized after
            // transformation.
            if (as<IRTargetIntrinsicDecoration>(decoration))
                return false;

            // Skip entry-point and pseudo-entry-point functions
            // since we cannot legalize the input parameters.
            if (as<IREntryPointDecoration>(decoration) || as<IRCudaKernelDecoration>(decoration) ||
                as<IRAutoPyBindCudaDecoration>(decoration))
                return false;

            // Skip functions with CudaDeviceExport decoration.
            // These functions have externally visible signatures that should not be changed.
            if (func->findDecorationImpl(kIROp_CudaDeviceExportDecoration))
                return false;
        }

        // Skip functions with `kIROp_GenericAsm` since
        // these instructions inject target specific code
        // using parameters in an unpredictable way, relying
        // on assumptions that parameters do not change type.
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (!as<IRGenericAsm>(inst))
                    continue;
                return false;
            }
        }

        return true;
    }

    // True if `func` has a use that requires its signature be preserved - i.e. a use other than as
    // the direct callee of an `IRCall` (for example, taken as a function value / callback). Such a
    // function cannot have its parameters rewritten, so `processFunc` leaves it untransformed.
    // True if `func` is used in a way that requires its signature to survive this pass unchanged -
    // anything other than being called or annotated. Three use kinds are treated as benign and do
    // not preserve the signature: decorations, specialization-dictionary entries (transient
    // bookkeeping for later specialization/finalization), and being the *callee* operand of a call.
    // Anything else - taken as a function value, passed as a callback - counts.
    //
    // Two callers depend on this. `processFunc` uses it to skip rewriting the function at all, and
    // `findForwardableParamStorage` uses it to decline forwarding an address into it: a function
    // whose signature is preserved keeps by-value parameters, so handing it a pointer would be an
    // ABI mismatch (see `cuda-forward-uniform-signature-preserved-callee.slang`). Widening the
    // benign set therefore relaxes a correctness decision, not just an optimization.
    bool hasSignaturePreservingUse(IRFunc* func)
    {
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (as<IRDecoration>(user))
                continue;
            // Specialization dictionary entries are transient bookkeeping for later
            // specialization/finalization and should not block borrow-in rewriting
            // of entry-point parameters.
            if (as<IRCompilerDictionaryValue>(user) || as<IRCompilerDictionaryEntry>(user))
                continue;
            if (auto call = as<IRCall>(user))
            {
                if (call->getCalleeUse() == use)
                    continue;
            }
            return true;
        }
        return false;
    }

    // Process a single function
    void processFunc(IRFunc* func)
    {
        HashSet<IRParam*> updatedParams;

        // If the function is used in any way that is not understood by the compiler (e.g. as a
        // callback), we must preserve its signature, so do not modify it.
        if (hasSignaturePreservingUse(func))
            return;

        // First pass: Transform parameter types
        for (auto param = func->getFirstParam(); param; param = param->getNextParam())
        {
            if (shouldTransformParam(param))
            {
                // Our goal here is to transform `in T` parameters to `borrow in T`.
                // We are selective about what we will transform for a few reasons:
                // 1. no reason to transform simple primitives like `int`.
                // 2. not every type makes sense as constref. For example, `ParameterBlock`.
                // 3. `borrow in` is not 100% stable, so we need to be selective on what we let
                //    transform into `borrow in`.
                //
                // This allows us to pass the address of variables directly into a function,
                // giving us the choice to remove copies into a parameter.
                auto paramType = param->getDataType();
                auto constRefType = builder.getBorrowInParamType(paramType, AddressSpace::Generic);
                param->setFullType(constRefType);

                changed = true;
                updatedParams.add(param);
            }
        }

        if (updatedParams.getCount() == 0)
        {
            return;
        }

        fixUpFuncType(func);

        // Second pass: Update function body according to the new `constref` parameters
        rewriteParamUseSitesToSupportConstRefUsage(updatedParams);

        // Third pass: Update call sites
        updateCallSites(func, updatedParams);
    }

    void addFuncsToCallListInTopologicalOrder(
        IRFunc* root,
        List<IRFunc*>& functionsToProcess,
        HashSet<IRFunc*>& visitedCandidates)
    {
        // We added 'root' already, leave
        if (visitedCandidates.contains(root))
            return;

        visitedCandidates.add(root);

        for (auto block : root->getBlocks())
        {
            for (auto blockInst : block->getChildren())
            {
                auto call = as<IRCall>(blockInst);
                if (!call)
                    continue;

                auto callee = as<IRFunc>(call->getCallee());
                if (!callee)
                    continue;

                addFuncsToCallListInTopologicalOrder(callee, functionsToProcess, visitedCandidates);
            }
        }

        if (!shouldProcessFunction(root))
            return;
        functionsToProcess.add(root);
    }

    SlangResult processModule()
    {
        // Collect all functions that need processing.
        // Process all callee's before callers; otherwise we introduce bugs

        HashSet<IRFunc*> visitedCandidates;
        List<IRFunc*> functionsToProcess;
        for (auto inst = module->getModuleInst()->getFirstChild(); inst; inst = inst->getNextInst())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;
            addFuncsToCallListInTopologicalOrder(func, functionsToProcess, visitedCandidates);
        }

        // Process each function
        for (auto func : functionsToProcess)
        {
            processFunc(func);
        }

        return SLANG_OK;
    }
};

SlangResult transformParamsToConstRef(
    IRModule* module,
    TargetRequest* targetReq,
    DiagnosticSink* sink)
{
    // The entry-point uniform address-forward (the #11774 fix) is specific to how CUDA lowers a
    // by-value kernel parameter, so it is enabled only for the CUDA family. Other targets keep the
    // temp-copy path, leaving their codegen unchanged.
    const bool forwardEntryPointUniformAddress = isCUDATarget(targetReq);
    TransformParamsToConstRefContext context(module, sink, forwardEntryPointUniformAddress);
    return context.processModule();
}

struct EntryPointInParamToBorrowContext : public TransformParamsToConstRefContext
{
    EntryPointInParamToBorrowContext(IRModule* module, DiagnosticSink* sink)
        : TransformParamsToConstRefContext(
              module,
              sink,
              /* forwardEntryPointUniformAddress */ false)
    {
    }
    virtual bool shouldProcessFunction(IRFunc* func) override
    {
        if (!func->isDefinition())
            return false;
        if (func->findDecoration<IREntryPointDecoration>() != nullptr)
            return true;
        return false;
    }
    virtual bool shouldTransformParam(IRParam* param) override
    {
        auto type = param->getDataType();
        if (as<IRPointerLikeType>(type))
            return false;
        if (as<IRPtrTypeBase>(type))
            return false;
        if (as<IRMeshOutputType>(type))
            return false;
        if (as<IRHLSLPatchType>(type))
            return false;

        // Skip uniform parameters.
        // We expect all entry-point parameters to have layout information,
        // but we will be defensive and skip parameters without the required
        // information when we are in a release build.
        //
        auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(layoutDecoration);
        if (!layoutDecoration)
            return false;
        auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
        SLANG_ASSERT(paramLayout);
        if (!paramLayout)
            return false;
        if (!isVaryingParameter(paramLayout))
            return false;

        // If we reach here, we are dealing with a varying in parameter.
        // We need to rewrite it to be a `borrow in` parameter.
        return true;
    }
};

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink)
{
    EntryPointInParamToBorrowContext context(module, sink);
    return context.processModule();
}

} // namespace Slang
