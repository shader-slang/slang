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
// Only applyVariableScopeCorrection and metadata collection run after this
// pass, neither of which creates new pointer types or inspects field types.
//
// ## Type translation consistency
//
// The pass rewrites struct fields in-place and updates all get_field_addr /
// rwstructuredBufferGetElementPtr result types.  Because the IR is post-SSA
// and post-inlining, every access site is a direct field-address + load/store
// sequence in the entry-point function body.  No cross-function propagation
// is needed.
//
// ## Instruction coverage
//
// Only load and store instructions on rewritten field addresses need casts.
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

        for (auto globalInst : module->getGlobalInsts())
        {
            auto structType = as<IRStructType>(globalInst);
            if (!structType)
                continue;

            bool usedInBuffer = false;
            for (auto use = structType->firstUse; use; use = use->nextUse)
            {
                auto user = use->getUser();
                if (as<IRConstantBufferType>(user) || as<IRParameterBlockType>(user) ||
                    as<IRHLSLStructuredBufferTypeBase>(user))
                {
                    usedInBuffer = true;
                    break;
                }
            }

            if (!usedInBuffer)
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
            }

            if (fieldsToLower.getCount() > 0)
                result[structType] = fieldsToLower;
        }

        return result;
    }

    void rewriteStructFields(Dictionary<IRStructType*, List<FieldLoweringInfo>>& structsToLower)
    {
        auto uintPtrType = getUIntPtrType();
        for (auto& [_, fields] : structsToLower)
        {
            for (auto& fieldInfo : fields)
                fieldInfo.field->setFieldType(uintPtrType);
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

                for (auto inst : fieldAddressInsts)
                {
                    auto fieldAddr = cast<IRFieldAddress>(inst);
                    auto ptrToField = as<IRPtrTypeBase>(fieldAddr->getFullType());
                    if (ptrToField)
                        fieldAddr->setFullType(builder.getPtrType(uintPtrType, ptrToField));

                    insertCastsForPointerUses(fieldAddr, fieldInfo.originalPtrType, uintPtrType);
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
            List<IRInst*> elemPtrInsts;
            for (auto use = bufType->firstUse; use; use = use->nextUse)
            {
                auto bufVar = use->getUser();
                for (auto varUse = bufVar->firstUse; varUse; varUse = varUse->nextUse)
                {
                    auto varUser = varUse->getUser();
                    if (varUser->getOp() == kIROp_RWStructuredBufferGetElementPtr)
                        elemPtrInsts.add(varUser);
                }
            }

            // Use replaceOperand to safely mutate the hoistable buffer type.
            builder.replaceOperand(bufType->getOperands() + 0, uintPtrType);

            for (auto inst : elemPtrInsts)
            {
                auto ptrType = as<IRPtrTypeBase>(inst->getFullType());
                if (ptrType)
                    inst->setFullType(builder.getPtrType(uintPtrType, ptrType));

                insertCastsForPointerUses(inst, origElemType, uintPtrType);
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
