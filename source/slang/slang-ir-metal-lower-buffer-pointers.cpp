#include "slang-ir-metal-lower-buffer-pointers.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

// Metal rejects pointer-to-pointer types in buffer pointee types.  When a
// struct bound via [[buffer(N)]] contains a pointer field, and the buffer
// binding itself is a device pointer, Metal sees `device T* device*` and
// rejects the shader.
//
// This pass rewrites pointer-typed struct fields to UIntPtr (which emits as
// `ulong` on Metal) and inserts CastIntToPtr/CastPtrToInt at access sites.
// It covers structs used in ConstantBuffer, ParameterBlock, and
// RWStructuredBuffer element types.
//
// For storage buffers (RWStructuredBuffer<T*>), the buffer's element type
// itself is a pointer, so the pass also rewrites the element type of the
// buffer and inserts casts at element-access sites.
//
// ## Pass ordering and safety
//
// This pass runs very late (after simplifyNonSSAIR, see slang-emit.cpp) so
// that all intermediate passes see real pointer types.  At this point:
//   - Address spaces are finalized (specializeAddressSpaceForMetal ran).
//   - All force-inlined functions have been expanded.
//   - Phi nodes are eliminated; the IR is in non-SSA form.
//   - Final simplification has removed dead code.
// After this pass: applyVariableScopeCorrection, collectCooperativeMetadata
// (conditional), and validateIRModuleIfEnabled. None create new pointer
// types or inspect field types.
//
// ## Type translation consistency
//
// The pass rewrites struct fields in-place and updates result types for
// FieldAddress, StructuredBufferLoad/Store, and GetElementPtr instructions.
// Buffer types are mutated via IRBuilder::replaceOperand to respect the
// global deduplication map for hoistable type nodes.
//
// Struct collection recursively walks nested struct fields — Metal's
// pointer-to-pointer restriction is transitive through nested types.
//
// ## Instruction coverage
//
// For struct fields: load and store on rewritten field addresses get casts.
// For storage buffer elements: Load, Store, and GetElementPtr are actively
// lowered; LoadStatus, Consume, and Append assert as unreachable (desugared
// by lowerAppendConsumeStructuredBuffers before this pass).
//
// ## Relationship to the early ParameterBlock pass
//
// The early pass (MetalParameterBlockElementTypeLoweringPolicy in
// slang-ir-lower-buffer-element-type.cpp) converts resource-typed fields
// (Texture2D, RWStructuredBuffer, etc.) in ParameterBlock structs into
// DescriptorHandle wrappers.  It must run early to prevent the resource
// legalization pass from hoisting resources out of argument buffer structs.
// It does NOT handle pointer fields — those pass through unchanged and
// are caught by this late pass via the IRParameterBlockType check in
// collectStructsToLower.
//
// ## Why UIntPtrType instead of UInt64Type
//
// UIntPtrType is semantically "pointer-sized unsigned integer" — it emits
// identically to UInt64Type on Metal (`ulong`) but communicates that the
// value holds a pointer address.  This prevents future optimization passes
// from accidentally treating the field as plain integer data.

static bool isMultiLevelPointer(IRType* type)
{
    if (auto ptrType = as<IRPtrType>(type))
        return as<IRPtrType>(ptrType->getValueType()) != nullptr;
    if (auto arrayType = as<IRArrayTypeBase>(type))
        return isMultiLevelPointer(arrayType->getElementType());
    return false;
}

struct MetalBufferPointerLoweringContext
{
    IRModule* module;
    IRBuilder builder;

    struct FieldLoweringInfo
    {
        IRStructField* field;
        IRStructKey* key;
        IRType* originalPtrType;
    };

    MetalBufferPointerLoweringContext(IRModule* inModule)
        : module(inModule), builder(inModule)
    {
    }

    IRType* getUIntPtrType() { return builder.getType(kIROp_UIntPtrType); }

    // --- Buffer struct field lowering ---

    Dictionary<IRStructType*, List<FieldLoweringInfo>> collectStructsToLower()
    {
        Dictionary<IRStructType*, List<FieldLoweringInfo>> result;

        HashSet<IRStructType*> bufferBoundStructs;
        for (auto globalInst : module->getGlobalInsts())
        {
            auto structType = as<IRStructType>(globalInst);
            if (!structType)
                continue;
            for (auto use = structType->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (as<IRConstantBufferType>(user) || as<IRParameterBlockType>(user) ||
                    as<IRHLSLStructuredBufferTypeBase>(user))
                {
                    bufferBoundStructs.add(structType);
                    break;
                }
            }
        }

        // Recursively walk all structs reachable from buffer-bound structs.
        // Metal's pointer-to-pointer restriction is transitive — a nested
        // struct with multi-level pointers also makes the outer type invalid.
        HashSet<IRStructType*> visited;
        List<IRStructType*> worklist;
        for (auto s : bufferBoundStructs)
            worklist.add(s);

        while (worklist.getCount() > 0)
        {
            auto structType = worklist.getLast();
            worklist.removeLast();
            if (!visited.add(structType))
                continue;

            List<FieldLoweringInfo> fieldsToLower;
            for (auto field : structType->getFields())
            {
                if (isMultiLevelPointer(field->getFieldType()))
                {
                    FieldLoweringInfo info;
                    info.field = field;
                    info.key = field->getKey();
                    info.originalPtrType = field->getFieldType();
                    fieldsToLower.add(info);
                }
                if (auto nestedStruct = as<IRStructType>(field->getFieldType()))
                    worklist.add(nestedStruct);
                else if (auto arrayType = as<IRArrayTypeBase>(field->getFieldType()))
                {
                    if (auto elemStruct = as<IRStructType>(arrayType->getElementType()))
                        worklist.add(elemStruct);
                }
                else if (auto ptrType = as<IRPtrType>(field->getFieldType()))
                {
                    if (auto pointeeStruct = as<IRStructType>(ptrType->getValueType()))
                        worklist.add(pointeeStruct);
                }
            }

            if (fieldsToLower.getCount() > 0)
                result[structType] = fieldsToLower;
        }

        return result;
    }

    IRType* getLoweredFieldType(IRType* originalType)
    {
        if (auto arrayType = as<IRArrayTypeBase>(originalType))
            return builder.getArrayType(
                getLoweredFieldType(arrayType->getElementType()),
                arrayType->getElementCount());
        return getUIntPtrType();
    }

    void rewriteStructFields(Dictionary<IRStructType*, List<FieldLoweringInfo>>& structsToLower)
    {
        for (auto& [_, fields] : structsToLower)
        {
            for (auto& fieldInfo : fields)
                fieldInfo.field->setFieldType(getLoweredFieldType(fieldInfo.originalPtrType));
        }
    }

    void insertCastsForStructFields(
        Dictionary<IRStructType*, List<FieldLoweringInfo>>& structsToLower)
    {
        auto uintPtrType = getUIntPtrType();

        for (auto& [_, fields] : structsToLower)
        {
            for (auto& fieldInfo : fields)
            {
                List<IRInst*> fieldAddressInsts;
                for (auto use = fieldInfo.key->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (user->getOp() == kIROp_FieldAddress)
                        fieldAddressInsts.add(user);
                }

                auto loweredFieldType = getLoweredFieldType(fieldInfo.originalPtrType);
                // Unwrap nested arrays to find the innermost pointer type.
                auto origElemPtrType = fieldInfo.originalPtrType;
                while (auto arr = as<IRArrayTypeBase>(origElemPtrType))
                    origElemPtrType = (IRType*)arr->getElementType();

                for (auto inst : fieldAddressInsts)
                {
                    auto fieldAddr = cast<IRFieldAddress>(inst);
                    auto ptrToField = as<IRPtrTypeBase>(fieldAddr->getFullType());
                    if (ptrToField)
                        fieldAddr->setFullType(builder.getPtrType(loweredFieldType, ptrToField));

                    insertCastsForPointerUses(fieldAddr, origElemPtrType, uintPtrType);
                }
            }
        }
    }

    // --- StorageBuffer element lowering ---

    void lowerStorageBufferElements()
    {
        auto uintPtrType = getUIntPtrType();

        for (auto globalInst : module->getGlobalInsts())
        {
            auto bufType = as<IRHLSLStructuredBufferTypeBase>(globalInst);
            if (!bufType)
                continue;
            auto elemType = bufType->getElementType();
            if (!as<IRPtrType>(elemType))
                continue;

            auto origElemType = elemType;

            // Collect element-access instructions BEFORE mutating the type,
            // so we only capture accesses from pointer-element buffers (not
            // a user-declared RWStructuredBuffer<ulong> that may merge after).
            List<IRInst*> elemAccessInsts;
            for (auto use = bufType->firstUse; use; use = use->nextUse)
            {
                auto bufVar = use->getUser();
                for (auto varUse = bufVar->firstUse; varUse; varUse = varUse->nextUse)
                    elemAccessInsts.add(varUse->getUser());
            }

            // Use replaceOperand to safely mutate the hoistable buffer type.
            builder.replaceOperand(bufType->getOperands() + 0, uintPtrType);

            for (auto inst : elemAccessInsts)
            {
                switch (inst->getOp())
                {
                case kIROp_RWStructuredBufferGetElementPtr:
                    {
                        auto ptrType = as<IRPtrTypeBase>(inst->getFullType());
                        if (ptrType)
                            inst->setFullType(builder.getPtrType(uintPtrType, ptrType));
                        insertCastsForPointerUses(inst, origElemType, uintPtrType);
                        break;
                    }
                case kIROp_StructuredBufferLoad:
                case kIROp_RWStructuredBufferLoad:
                    {
                        inst->setFullType(uintPtrType);
                        builder.setInsertAfter(inst);
                        auto castInst = builder.emitCastIntToPtr(origElemType, inst);
                        inst->replaceUsesWith(castInst);
                        castInst->setOperand(0, inst);
                        break;
                    }
                case kIROp_RWStructuredBufferStore:
                    {
                        auto val = inst->getOperand(2);
                        builder.setInsertBefore(inst);
                        IRInst* args[] = {val};
                        auto castInst =
                            builder.emitIntrinsicInst(uintPtrType, kIROp_CastPtrToInt, 1, args);
                        inst->setOperand(2, castInst);
                        break;
                    }
                // These ops are desugared by lowerAppendConsumeStructuredBuffers
                // before this pass runs. Assert rather than silently lowering.
                case kIROp_StructuredBufferLoadStatus:
                case kIROp_RWStructuredBufferLoadStatus:
                case kIROp_StructuredBufferConsume:
                case kIROp_StructuredBufferAppend:
                    SLANG_UNREACHABLE("structured buffer op should be desugared before this pass");
                    break;
                default:
                    break;
                }
            }
        }
    }

    // --- Shared cast insertion logic ---

    void insertCastsForPointerUses(
        IRInst* addressInst,
        IRType* originalPtrType,
        IRType* uintPtrType)
    {
        List<IRInst*> usesToProcess;
        for (auto use = addressInst->firstUse; use; use = use->nextUse)
            usesToProcess.add(use->getUser());

        for (auto user : usesToProcess)
        {
            if (user->getOp() == kIROp_Load)
            {
                user->setFullType(uintPtrType);
                builder.setInsertAfter(user);
                auto castInst = builder.emitCastIntToPtr(originalPtrType, user);
                user->replaceUsesWith(castInst);
                castInst->setOperand(0, user);
            }
            else if (user->getOp() == kIROp_Store)
            {
                auto storeInst = cast<IRStore>(user);
                auto val = storeInst->getVal();
                builder.setInsertBefore(storeInst);
                IRInst* args[] = {val};
                auto castInst = builder.emitIntrinsicInst(uintPtrType, kIROp_CastPtrToInt, 1, args);
                storeInst->setOperand(1, castInst);
            }
            else if (user->getOp() == kIROp_GetElementPtr)
            {
                // Array indexing through the field address — update the GEP
                // result type and recurse into its uses.
                auto ptrToElem = as<IRPtrTypeBase>(user->getFullType());
                if (ptrToElem)
                    user->setFullType(builder.getPtrType(uintPtrType, ptrToElem));
                insertCastsForPointerUses(user, originalPtrType, uintPtrType);
            }
            else
            {
                SLANG_ASSERT(!"Unexpected user of lowered pointer field address");
            }
        }
    }

    void processModule()
    {
        auto structsToLower = collectStructsToLower();
        if (structsToLower.getCount() > 0)
        {
            rewriteStructFields(structsToLower);
            insertCastsForStructFields(structsToLower);
        }

        lowerStorageBufferElements();
    }
};

void lowerMetalBufferPointerTypes(IRModule* module)
{
    MetalBufferPointerLoweringContext context(module);
    context.processModule();
}

} // namespace Slang
