#include "slang-ir-defer-buffer-load.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Generally, we want to specialize arguments that are large in size, or arguments that
// are arrays or composite type that contains arrays.
// This is because:
// 1. Struct types without arrays will eventually be SROA's into registers and then effectively
//    DCE'd, so they usually won't cause performance issues. In fact, front loading structs
//    and reusing the loaded value instead of repetitively loading from constant memory is
//    usually beneficial to performance. However large struct values can be SROA'd into a large
//    number of registers, causing slow downstream compilation. Therefore we should avoid/defer
//    loading them into registers if we can.
// 2. Arrays usually cannot be SROA'd into individual registers, which usually leads to
//    large register consumption if they ever get loaded, so we want to defer loading array
//    typed values as much as possible.

// If the argument data is bigger than this threshold, it is considered a large object
// and we will try to specialize it even if it doesn't contain arrays.
static const int kBufferLoadElementSizeSpecializationThreshold = 128;

// If the argument data is smaller than this threshold, it is considered a tiny object
// and we will not consider specializing it, even if it contains arrays.
static const int kBufferLoadElementSizeSpecializationMinThreshold = 16;

static bool isCompositeTypeContainingArrays(IRType* type)
{
    if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            if (const auto arrayType = as<IRArrayTypeBase>(field->getFieldType()))
            {
                return true;
            }
            if (auto subStructType = as<IRStructType>(field->getFieldType()))
            {
                if (isCompositeTypeContainingArrays(subStructType))
                    return true;
            }
        }
    }
    else if (as<IRArrayTypeBase>(type))
    {
        return true;
    }
    return false;
}

bool isTypePreferrableToDeferLoad(CodeGenContext* codeGenContext, IRType* type)
{
    // If parameter is a pointer/reference, we should consider specialize it.
    if (as<IROutParamTypeBase>(type) || as<IRRefParamType>(type) || as<IRBorrowInParamType>(type))
        return true;

    // We only want to defer loading values that are "large enough" that
    // we expect them to be expensive to pass by value.
    //
    IRSizeAndAlignment sizeAlignment = {};
    if (SLANG_FAILED(getNaturalSizeAndAlignment(
            codeGenContext->getTargetProgram()->getOptionSet(),
            type,
            &sizeAlignment)))
    {
        // If type contains fields that we don't know how to compute natural size
        // for, default to specialize if it contains arrays.
        return isCompositeTypeContainingArrays(type);
    }

    // If the argument is very small, don't bother specializing.
    if (sizeAlignment.size <= kBufferLoadElementSizeSpecializationMinThreshold)
        return false;

    // If the argument is somewhat small, don't specialize, unless it contains
    // arrays.
    if (sizeAlignment.size <= kBufferLoadElementSizeSpecializationThreshold)
    {
        // We generally do not specialize for small values, except it contains
        // arrays that usually present a challenge for the SROA pass to eliminate
        // unnecessary loads.
        if (!isCompositeTypeContainingArrays(type))
            return false;
    }
    return true;
}

// Returns true if memory loaded by `loadInst` is not modified before `userInst` after it is
// loaded.
// This method is currently implementing a very conservative analysis that only allows
// `loadInst` to be in the same block as `userInst`, with basic aliasing analysis for any
// stores in between. All other cases are conservatively treated as the memory location may be
// modified.
bool isMemoryLocationUnmodifiedBetweenLoadAndUser(
    TargetRequest* target,
    IRInst* loadInst,
    IRInst* userInst)
{
    auto func = getParentFunc(loadInst);
    if (!func)
        return false;

    // For now we only check if loadInst and userInst are in the same block.
    if (loadInst->getParent() != userInst->getParent())
        return false;

    for (IRInst* inst = loadInst->getNextInst(); inst; inst = inst->getNextInst())
    {
        // We found callInst before hitting any instruction that may modify the memory.
        if (inst == userInst)
            return true;

        if (!inst->mightHaveSideEffects())
            continue;

        // If we see any inst that has side effect, check if it is simple case that we can rule
        // out the possibility of modifying the memory location.
        switch (inst->getOp())
        {
        case kIROp_Store:
            {
                auto storedDest = inst->getOperand(0);
                if (canAddressesPotentiallyAlias(target, func, loadInst->getOperand(0), storedDest))
                    return false;
                continue;
            }
        default:
            // For any other case, conservatively assume the memory location may be modified.
            return false;
        }
    }
    // We didn't found callInst after loadInst within the same basic block.
    // We conservatively assume the memory location may be modified.
    // This check can be extended to use the dominator tree to allow
    // loadInst and userInst to be in different blocks.
    return false;
}

struct DeferBufferLoadContext
{
    CodeGenContext* codeGenContext;


    void deferBufferLoadInst(IRBuilder& builder, List<IRInst*>& workList, IRInst* loadInst)
    {
        bool failDueToAttributeFound = false;
        for (auto attr : loadInst->getAllAttrs())
        {
            if (as<IRAlignedAttr>(attr) || as<IRMemoryScopeAttr>(attr))
            {
                failDueToAttributeFound = true;
                break;
            }
        }

        // Don't defer the load anymore if the type is simple.
        if (failDueToAttributeFound ||
            !isTypePreferrableToDeferLoad(codeGenContext, loadInst->getDataType()))
        {
            return;
        }

        auto rootAddr = getRootAddr(loadInst->getOperand(0));
        bool isImmutableBufferLoad = isPointerToImmutableLocation(rootAddr);

        // Don't defer the load if there are uses that are not getElement or fieldExtract.
        // Because in this case we need to use the entire loaded value, and further deferring
        // the load down any access chain will introduce redundant loads.
        for (auto use = loadInst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            switch (user->getOp())
            {
            case kIROp_GetElement:
            case kIROp_FieldExtract:
                // Can we defer the load to load only the requested element right before
                // the element extract inst?
                // If the buffer is immutable, we can always do that.
                // If it is not, we need to make sure there is no other instructions that can modify
                // the buffer between the load and the use.
                //
                if (isImmutableBufferLoad)
                    continue;
                if (isMemoryLocationUnmodifiedBetweenLoadAndUser(
                        codeGenContext->getTargetReq(),
                        loadInst,
                        user))
                    continue;
                return;
            default:
                // If we see any other use the laod instruction, we assume the entire loaded value
                // is needed, and we can't defer the load anymore.
                return;
            }
        }

        // If we reach here, it means all uses are getElement or fieldExtract, and
        // it is safe to defer the load down the access chain.

        if (loadInst->getOp() == kIROp_StructuredBufferLoad)
        {
            // Convert the structuredBufferLoad to a regular load to reuse
            // the same logic for deferring regular loads.
            builder.setInsertBefore(loadInst);
            auto bufferPtr = builder.emitRWStructuredBufferGetElementPtr(
                loadInst->getOperand(0),
                loadInst->getOperand(1));
            auto sbLoad = builder.emitLoad(bufferPtr);
            loadInst->transferDecorationsTo(sbLoad);
            loadInst->replaceUsesWith(sbLoad);
            loadInst->removeAndDeallocate();
            loadInst = sbLoad;
        }

        // Otherwise, look for all uses and try to defer the load before actual use of the value.
        ShortList<IRInst*> pendingWorkList;
        bool needMaterialize = false;
        traverseUses(
            loadInst,
            [&](IRUse* use)
            {
                auto user = use->getUser();

                switch (user->getOp())
                {
                case kIROp_GetElement:
                case kIROp_FieldExtract:
                    {
                        // If we see a getElement or fieldExtract, we defer the load by
                        // replacing the getElement/fieldExtract with a load of the
                        // elementAddr/fieldAddr.
                        builder.setInsertBefore(user);
                        auto basePtr = loadInst->getOperand(0);
                        IRInst* gepArg = user->getOperand(1);
                        auto elementPtr = builder.emitElementAddress(
                            basePtr,
                            makeArrayViewSingle<IRInst*>(gepArg));
                        auto newLoad = builder.emitLoad(elementPtr);
                        user->transferDecorationsTo(newLoad);
                        user->replaceUsesWith(newLoad);
                        user->removeAndDeallocate();

                        // Now add the new load to work list to try to defer it further.
                        pendingWorkList.add(newLoad);
                    }
                    break;
                default:
                    needMaterialize = true;
                    return;
                }
            });

        // Append to worklist in reverse order so we process the uses in natural appearance
        // order.
        for (Index i = pendingWorkList.getCount() - 1; i >= 0; i--)
            workList.add(pendingWorkList[i]);
    }

    void deferBufferLoadInFunc(IRFunc* func)
    {
        removeRedundancyInFunc(func, false);

        List<IRInst*> workList;

        // Discover all load instructions and add to work list.

        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                switch (inst->getOp())
                {
                case kIROp_Load:
                case kIROp_StructuredBufferLoad:
                    // Note: We don't handle `kIROp_StructuredBufferLoadStatus` here because
                    // it also writes to the status code out parameter, which we can't defer.
                    workList.add(inst);
                    break;
                }
            }
        }

        // Iteratively process the work list until it is empty.
        IRBuilder builder(func);
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto inst = workList[i];
            deferBufferLoadInst(builder, workList, inst);
        }
    }

    void deferBufferLoad(IRGlobalValueWithCode* inst)
    {
        if (auto func = as<IRFunc>(inst))
        {
            deferBufferLoadInFunc(func);
        }
        else if (auto generic = as<IRGeneric>(inst))
        {
            auto inner = findGenericReturnVal(generic);
            if (auto innerFunc = as<IRFunc>(inner))
                deferBufferLoadInFunc(innerFunc);
        }
    }
};

void deferBufferLoad(CodeGenContext* codeGenContext, IRModule* module)
{
    DeferBufferLoadContext context;
    context.codeGenContext = codeGenContext;
    for (auto childInst : module->getGlobalInsts())
    {
        if (auto code = as<IRGlobalValueWithCode>(childInst))
        {
            context.deferBufferLoad(code);
        }
    }
}

} // namespace Slang
