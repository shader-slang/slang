#include "slang-ir-insert-debug-value-store.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
void expandDebugValuePackBindings(IRInst* packValue, ArrayView<IRInst*> elements)
{
    // Consider `out expand each T values` specialized with T = (uint, float).
    // Parameter expansion produces two reference parameters. Bind those references to the
    // source pack's element fragments directly, so debug metadata does not require a runtime
    // aggregate containing OutParam values. The fragment types retain each scalar parameter's
    // reference convention, just like an ordinary scalar reference parameter's debug binding.
    List<IRDebugValue*> bindings;
    for (auto use = packValue->firstUse; use; use = use->nextUse)
        if (auto binding = as<IRDebugValue>(use->getUser()))
            if (binding->getValue() == packValue)
                bindings.add(binding);

    IRBuilder builder(packValue);
    for (auto binding : bindings)
    {
        builder.setInsertBefore(binding);
        for (Index i = 0; i < elements.getCount(); ++i)
        {
            auto element = elements[i];
            auto fragment = builder.emitElementAddress(
                builder.getPtrType(element->getFullType()),
                binding->getDebugVar(),
                builder.getIntValue(builder.getIntType(), i));
            auto update = builder.emitDebugValue(fragment, element);
            copyNameHintAndDebugDecorations(update, binding);
        }
        binding->removeAndDeallocate();
    }
}

/// Reuses the types of the original address chain when describing a variable fragment.
/// Consider a store to `Pair<uint>.x` before specialization: lowering already computed the field
/// address type, while rediscovering it from the generic struct would require specialization.
static void emitDebugValueUpdate(
    IRBuilder& builder,
    IRInst* debugVar,
    IRInst* newValue,
    const List<IRInst*>& accessChain,
    const List<IRInst*>& accessTypes)
{
    auto ptr = builder.emitElementAddress(
        debugVar,
        accessChain.getArrayView(),
        accessTypes.getArrayView());
    builder.emitDebugValue(ptr, newValue);
}

/// Records source variables before SSA promotion erases their storage and assignments.
/// All declared types participate; specialization and target lowering preserve their
/// representation.
static void insertDebugValueStore(IRFunc* func)
{
    IRBuilder builder(func);
    Dictionary<IRInst*, IRInst*> mapVarToDebugVar;
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;
    auto funcDebugLoc = func->findDecoration<IRDebugLocationDecoration>();
    if (!funcDebugLoc)
        return;
    auto funcDebugDecor = func->findDecoration<IRDebugFuncDecoration>();
    SLANG_RELEASE_ASSERT(funcDebugDecor && "function has DebugLocation but no DebugFunc");
    auto debugFunc = funcDebugDecor->getDebugFunc();
    List<IRInst*> params;
    for (auto param : firstBlock->getParams())
    {
        params.add(param);
    }
    Index paramIndex = 0;
    for (auto param : params)
    {
        builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
        auto paramType = param->getDataType();
        auto debugVar = builder.emitDebugVar(
            paramType,
            funcDebugLoc->getSource(),
            funcDebugLoc->getLine(),
            funcDebugLoc->getCol(),
            debugFunc,
            builder.getIntValue(builder.getUIntType(), paramIndex));
        copyNameHintAndDebugDecorations(debugVar, param);

        // Reference parameters describe storage, so their debug value is the address itself.
        // For `void update(__ref uint value)`, a write to `value` changes the pointee but does
        // not replace this binding with a uint snapshot. This also avoids reading an `out`
        // parameter before the callee initializes it.
        if (as<IROutParamTypeBase>(paramType) || as<IRBorrowInParamType>(paramType) ||
            as<IRRefParamType>(paramType))
        {
            builder.emitDebugValue(debugVar, param);
            paramIndex++;
            continue;
        }

        mapVarToDebugVar[param] = debugVar;

        // Map any in-param proxy vars to the debug var.
        bool hasProxyVar = false;
        for (auto use = param->firstUse; use; use = use->nextUse)
        {
            if (auto inParamProxyVarDecor = as<IRInParamProxyVarDecoration>(use->getUser()))
            {
                mapVarToDebugVar[inParamProxyVarDecor->parent] = debugVar;
                hasProxyVar = true;
            }
        }

        if (!hasProxyVar)
        {
            builder.emitDebugValue(debugVar, param);
        }
        paramIndex++;
    }

    for (auto block : func->getBlocks())
    {
        IRInst* nextInst = nullptr;
        for (auto inst = block->getFirstInst(); inst; inst = nextInst)
        {
            nextInst = inst->getNextInst();
            if (auto varInst = as<IRVar>(inst))
            {
                if (auto debugLoc = varInst->findDecoration<IRDebugLocationDecoration>())
                {
                    auto varType = tryGetPointedToType(&builder, varInst->getDataType());
                    builder.setInsertBefore(varInst);
                    auto debugVar = builder.emitDebugVar(
                        varType,
                        debugLoc->getSource(),
                        debugLoc->getLine(),
                        debugLoc->getCol(),
                        debugLoc->getScope());
                    copyNameHintAndDebugDecorations(debugVar, varInst);
                    mapVarToDebugVar[varInst] = debugVar;
                }
            }
        }
    }

    // Collect all stores and insert debug value insts to update debug vars.

    for (auto block : func->getBlocks())
    {
        IRInst* nextInst = nullptr;
        for (auto inst = block->getFirstInst(); inst; inst = nextInst)
        {
            nextInst = inst->getNextInst();

            if (auto storeInst = as<IRStore>(inst))
            {
                List<IRInst*> accessChain;
                List<IRInst*> accessTypes;
                auto varInst = getRootAddr(storeInst->getPtr(), accessChain, &accessTypes);
                IRInst* debugVar = nullptr;
                if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                {
                    builder.setInsertAfter(storeInst);
                    emitDebugValueUpdate(
                        builder,
                        debugVar,
                        storeInst->getVal(),
                        accessChain,
                        accessTypes);
                }
            }
            else if (auto swizzledStore = as<IRSwizzledStore>(inst))
            {
                List<IRInst*> accessChain;
                List<IRInst*> accessTypes;
                auto varInst = getRootAddr(swizzledStore->getDest(), accessChain, &accessTypes);
                IRInst* debugVar = nullptr;
                if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                {
                    builder.setInsertAfter(swizzledStore);
                    auto loadVal = builder.emitLoad(swizzledStore->getDest());
                    emitDebugValueUpdate(builder, debugVar, loadVal, accessChain, accessTypes);
                }
            }
            else if (auto callInst = as<IRCall>(inst))
            {
                auto funcValue = getResolvedInstForDecorations(callInst->getCallee());
                if (!funcValue)
                    continue;
                for (UInt i = 0; i < callInst->getArgCount(); i++)
                {
                    auto arg = callInst->getArg(i);
                    if (!as<IRPtrTypeBase>(arg->getDataType()))
                        continue;
                    List<IRInst*> accessChain;
                    List<IRInst*> accessTypes;
                    auto varInst = getRootAddr(arg, accessChain, &accessTypes);
                    IRInst* debugVar = nullptr;
                    if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                    {
                        builder.setInsertAfter(callInst);
                        auto loadVal = builder.emitLoad(arg);
                        emitDebugValueUpdate(builder, debugVar, loadVal, accessChain, accessTypes);
                    }
                }
            }
        }
    }
}

void insertDebugValueStore(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (auto genericInst = as<IRGeneric>(globalInst))
        {
            if (auto func = as<IRFunc>(findGenericReturnVal(genericInst)))
            {
                insertDebugValueStore(func);
            }
        }
        else if (auto func = as<IRFunc>(globalInst))
        {
            insertDebugValueStore(func);
        }
    }
}
} // namespace Slang
