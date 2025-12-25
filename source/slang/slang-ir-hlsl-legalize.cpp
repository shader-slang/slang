// slang-ir-hlsl-legalize.cpp
#include "slang-ir-hlsl-legalize.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

#include <functional>

namespace Slang
{

void searchChildrenForForceVarIntoStructTemporarily(IRModule* module, IRInst* inst)
{
    for (auto child : inst->getChildren())
    {
        switch (child->getOp())
        {
        case kIROp_Block:
            {
                searchChildrenForForceVarIntoStructTemporarily(module, child);
                break;
            }
        case kIROp_Call:
            {
                auto call = as<IRCall>(child);
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    auto arg = call->getArg(i);
                    const bool isForcedStruct = arg->getOp() == kIROp_ForceVarIntoStructTemporarily;
                    const bool isForcedRayPayloadStruct =
                        arg->getOp() == kIROp_ForceVarIntoRayPayloadStructTemporarily;
                    if (!(isForcedStruct || isForcedRayPayloadStruct))
                        continue;

                    // Safety check: ensure the wrapper has an operand
                    if (arg->getOperandCount() == 0)
                        continue;

                    auto forceStructArg = arg->getOperand(0);
                    if (!forceStructArg)
                        continue;

                    auto dataType = forceStructArg->getDataType();
                    if (!dataType)
                        continue;

                    // For pointer types, extract the pointed-to type
                    IRType* forceStructBaseType = nullptr;
                    if (auto ptrType = as<IRPtrTypeBase>(dataType))
                    {
                        forceStructBaseType = ptrType->getValueType();
                    }
                    else if (dataType->getOperandCount() > 0)
                    {
                        forceStructBaseType = (IRType*)(dataType->getOperand(0));
                    }

                    if (!forceStructBaseType)
                        continue;

                    IRBuilder builder(call);
                    if (forceStructBaseType->getOp() == kIROp_StructType)
                    {
                        // The wrapped value is already a struct, just unwrap it
                        // Replace the wrapper with the direct argument
                        arg->replaceUsesWith(forceStructArg);
                        arg->removeAndDeallocate();
                        continue;
                    }

                    // When `__forceVarIntoStructTemporarily` is called with a non-struct type
                    // parameter, we create a temporary struct and copy the parameter into the
                    // struct. This struct is then subsituted for the return of
                    // `__forceVarIntoStructTemporarily`. Optionally, if
                    // `__forceVarIntoStructTemporarily` is a parameter to a side effect type
                    // (`ref`, `out`, `inout`) we copy the struct back into our original non-struct
                    // parameter.

                    const auto typeNameHint = isForcedRayPayloadStruct
                                                  ? "RayPayload_t"
                                                  : "ForceVarIntoStructTemporarily_t";
                    const auto varNameHint =
                        isForcedRayPayloadStruct ? "rayPayload" : "forceVarIntoStructTemporarily";

                    builder.setInsertBefore(call->getCallee());
                    auto structType = builder.createStructType();
                    StringBuilder structName;
                    builder.addNameHintDecoration(structType, UnownedStringSlice(typeNameHint));
                    if (isForcedRayPayloadStruct)
                        builder.addRayPayloadDecoration(structType);

                    auto elementBufferKey = builder.createStructKey();
                    builder.addNameHintDecoration(elementBufferKey, UnownedStringSlice("data"));
                    auto _dataField = builder.createStructField(
                        structType,
                        elementBufferKey,
                        forceStructBaseType);

                    builder.setInsertBefore(call);
                    auto structVar = builder.emitVar(structType);
                    builder.addNameHintDecoration(structVar, UnownedStringSlice(varNameHint));
                    builder.emitStore(
                        builder.emitFieldAddress(
                            builder.getPtrType(_dataField->getFieldType()),
                            structVar,
                            _dataField->getKey()),
                        builder.emitLoad(forceStructArg));

                    arg->replaceUsesWith(structVar);
                    arg->removeAndDeallocate();

                    // Check if we need to copy back (for out/inout parameters)
                    // Use the correct API to get parameter type
                    auto callee = call->getCallee();
                    auto funcType = as<IRFuncType>(callee->getDataType());
                    if (funcType && i < funcType->getParamCount())
                    {
                        auto paramType = funcType->getParamType(i);
                        // Only copy back if the parameter is a pointer-like type (out/inout/ref)
                        if (!isPtrLikeOrHandleType(paramType))
                            continue;

                        builder.setInsertAfter(call);
                        builder.emitStore(
                            forceStructArg,
                            builder.emitFieldAddress(
                                builder.getPtrType(_dataField->getFieldType()),
                                structVar,
                                _dataField->getKey()));
                    }
                }
                break;
            }
        }
    }
}

void legalizeNonStructParameterToStructForHLSL(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        // Search all global instructions for ForceVarIntoStructTemporarily wrappers.
        // These can appear in functions, generics, or other structures.
        searchChildrenForForceVarIntoStructTemporarily(module, globalInst);
    }
}

void legalizeEmptyRayPayloadsForHLSL(IRModule* module)
{
    // DXIL/HLSL with NVAPI requires non-empty ray payload structs because
    // the NvInvokeHitObject macro expects a Payload argument.
    IRBuilder builder(module);

    for (auto globalInst : module->getGlobalInsts())
    {
        auto structType = as<IRStructType>(globalInst);
        if (!structType)
            continue;

        // Check if this struct has ray payload decoration
        auto rayPayloadDec = structType->findDecoration<IRRayPayloadDecoration>();
        auto vulkanRayPayloadDec = structType->findDecoration<IRVulkanRayPayloadDecoration>();
        bool isRayPayload = rayPayloadDec != nullptr || vulkanRayPayloadDec != nullptr;

        if (!isRayPayload)
            continue;

        // Check if the struct is empty (has no fields)
        if (structType->getFields().begin() != structType->getFields().end())
            continue;

        // Add a dummy field to the empty ray payload struct
        // Insert the key BEFORE the struct type so it's defined before being referenced
        builder.setInsertBefore(structType);
        auto dummyKey = builder.createStructKey();
        builder.addNameHintDecoration(dummyKey, UnownedStringSlice("_slang_dummy"));

        // Add stage access decorations that ray payload fields require
        // This matches what would be generated for: int _slang_dummy : read(caller) :
        // write(caller);
        IRInst* stageName = builder.getStringValue(UnownedStringSlice("caller"));
        builder.addDecoration(dummyKey, kIROp_StageReadAccessDecoration, &stageName, 1);
        builder.addDecoration(dummyKey, kIROp_StageWriteAccessDecoration, &stageName, 1);

        builder.createStructField(structType, dummyKey, builder.getIntType());

        // Now find and update all makeStruct instructions that create this struct type.
        // Since we added a field, we need to add a default value (0) as an operand.
        // Collect uses first to avoid modifying the list while iterating
        List<IRInst*> makeStructsToUpdate;
        for (auto use = structType->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (user->getOp() == kIROp_MakeStruct && user->getDataType() == structType)
            {
                makeStructsToUpdate.add(user);
            }
        }

        for (auto makeStructInst : makeStructsToUpdate)
        {
            // Add a default value (0) for the new field
            builder.setInsertBefore(makeStructInst);
            auto defaultValue = builder.getIntValue(builder.getIntType(), 0);
            auto newMakeStruct = builder.emitMakeStruct(structType, 1, &defaultValue);
            makeStructInst->replaceUsesWith(newMakeStruct);
            makeStructInst->removeAndDeallocate();
        }
    }
}

} // namespace Slang
