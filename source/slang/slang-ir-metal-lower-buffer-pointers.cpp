#include "slang-ir-metal-lower-buffer-pointers.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
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
// StructuredBuffer-family (including RW/Append/Consume) element types.
//
// For storage buffers (RWStructuredBuffer<T*>), the buffer's element type
// itself is a pointer, so the pass also rewrites the element type of the
// buffer and inserts casts at element-access sites.
//
// ## Pass ordering and safety
//
// This pass runs very late (after simplifyNonSSAIR, see slang-emit.cpp) so
// that all intermediate passes see real pointer types.  At this point:
//   - Address spaces are propagated (specializeAddressSpaceForMetal ran).
//   - Functions marked [ForceInline] have been expanded; other helpers may
//     survive but are handled correctly via module-wide use-chain traversal.
//   - Phi nodes are eliminated (eliminatePhis ran); the IR is in non-SSA form.
//   - Final simplification has removed dead code.
// After this pass: applyVariableScopeCorrection, collectCooperativeMetadata
// (conditional), and validateIRModuleIfEnabled. None create new pointer
// types or inspect field types.
//
// ## Type translation consistency
//
// The pass rewrites struct fields in-place and updates result types for
// FieldAddress, FieldExtract (by-value access), GetElementPtr (arrays),
// GetOffsetPtr (pointer arithmetic on array element addresses),
// GetElement (by-value array indexing after FieldExtract on array fields),
// and StructuredBufferLoad/Store instructions. Buffer types are mutated
// via IRBuilder::replaceOperand to respect the global deduplication map
// for hoistable type nodes.
//
// Struct collection recursively walks nested struct fields — Metal's
// pointer-to-pointer restriction is transitive through nested types.
//
// ## Instruction coverage
//
// For struct fields: load and store on rewritten field addresses get casts.
// For storage buffer elements: Load, Store, and GetElementPtr are actively
// lowered; Consume and Append assert as unreachable (desugared by
// lowerAppendConsumeStructuredBuffers before this pass). LoadStatus asserts
// unreachable because it is HLSL-only ([require(hlsl)] in hlsl.meta.slang)
// and cannot appear in Metal shaders.
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

    // Metal's pointer-to-pointer restriction ONLY applies to buffer pointee
    // types — structs bound via [[buffer(N)]] as device* or constant*.
    // Structs used in threadgroup memory, thread-local variables, or function
    // parameters are free to contain int** — Metal accepts those. We therefore
    // seed the worklist only from structs directly referenced by buffer types.
    //
    // Note: because we mutate the struct field type in-place (not cloning),
    // if the same IRStructType is shared between a buffer binding and a
    // non-buffer context (e.g. groupshared), the lowering affects both.
    // This is safe because CastIntToPtr/CastPtrToInt are inserted at ALL
    // access sites (reached via the field key's use chain), so the round-trip
    // is consistent regardless of which address space the struct lives in.
    //
    // No layout-rule-aware type synthesis is needed:
    // - Pointer values are bit-exact device addresses (always 8 bytes).
    // - No padding/alignment variance across std140/std430/argument buffers.
    // - A single UIntPtr representation suffices for all buffer layouts.
    //
    // No multi-address-space scanning is needed:
    // - By this stage, only buffer bindings produce pointer-to-pointer in
    //   their pointee type.
    // - Address spaces are propagated (specializeAddressSpaceForMetal ran);
    //   buffer pointee types are the only source of the restriction.
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
                // Drill through arbitrary chains of arrays and pointers to
                // find nested structs. Metal's restriction is transitive
                // through all of: Struct, Array(Struct), Ptr(Struct),
                // Array(Array(Struct)), Array(Ptr(Struct)), etc.
                {
                    auto t = (IRType*)field->getFieldType();
                    for (;;)
                    {
                        if (auto s = as<IRStructType>(t))
                        {
                            worklist.add(s);
                            break;
                        }
                        if (auto a = as<IRArrayTypeBase>(t))
                        {
                            t = (IRType*)a->getElementType();
                            continue;
                        }
                        if (auto p = as<IRPtrType>(t))
                        {
                            t = (IRType*)p->getValueType();
                            continue;
                        }
                        break;
                    }
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

    // In-place field type mutation is sound because:
    //
    // 1. The struct key's firstUse chain reaches EVERY FieldAddress and
    //    FieldExtract instruction in the module that references this field.
    //    insertCastsForStructFields walks that chain and patches all sites.
    //
    // 2. At this late pipeline stage (post-eliminatePhis, non-SSA form),
    //    struct fields are accessed only via FieldAddress (→ Load/Store/GEP)
    //    or FieldExtract (by-value). The key's use chain is module-wide —
    //    it reaches FieldAddress/FieldExtract in all functions (entry points
    //    and any surviving non-inlined helpers alike).
    //
    // 3. Varying inputs (stage IO) are legalized by legalizeIRForMetal
    //    before this pass. They don't carry buffer-bound struct types —
    //    stage IO uses different binding mechanisms than [[buffer(N)]].
    //
    // 4. Global variables with these struct types appear as buffer bindings
    //    (caught by the seed set in collectStructsToLower).
    //
    // 5. No function specialization or cloning is needed:
    //    - The struct key use chain is module-wide, so FieldAddress/FieldExtract
    //      instructions inside any surviving helper functions are also patched.
    //    - No call sites to propagate rewritten types across — in-place type
    //      mutation is visible to all code referencing the struct type.
    //    - No linkage decorations to reconcile between clones.
    //    - No need for deferred pseudo-ops (CastStorageToLogical-style) —
    //      we insert final casts directly at each use site.
    void rewriteStructFields(Dictionary<IRStructType*, List<FieldLoweringInfo>>& structsToLower)
    {
        for (auto& [_, fields] : structsToLower)
        {
            for (auto& fieldInfo : fields)
                fieldInfo.field->setFieldType(getLoweredFieldType(fieldInfo.originalPtrType));
        }
    }

    // Only FieldAddress and FieldExtract reference struct keys in the IR.
    // Other ops that might be expected here:
    // - Call: does not reference struct keys; struct values passed by value
    //   are accessed via FieldAddress/FieldExtract inside the callee, which
    //   are already on the key's module-wide use chain.
    // - GetOffsetPtr: operates on pointer values, not struct field keys.
    //   (It CAN appear as a user of a GEP/FieldAddress result via pointer
    //   arithmetic — handled in insertCastsForPointerUses alongside GEP.)
    // - Return: struct return values are accessed via FieldExtract at the
    //   call site (already handled above).
    void insertCastsForStructFields(
        Dictionary<IRStructType*, List<FieldLoweringInfo>>& structsToLower)
    {
        auto uintPtrType = getUIntPtrType();

        for (auto& [_, fields] : structsToLower)
        {
            for (auto& fieldInfo : fields)
            {
                List<IRInst*> fieldAddressInsts;
                List<IRInst*> fieldExtractInsts;
                for (auto use = fieldInfo.key->firstUse; use; use = use->nextUse)
                {
                    auto user = use->getUser();
                    if (user->getOp() == kIROp_FieldAddress)
                        fieldAddressInsts.add(user);
                    else if (user->getOp() == kIROp_FieldExtract)
                        fieldExtractInsts.add(user);
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

                // FieldExtract returns the field value directly (by-value access,
                // e.g. when a struct is returned from a function). For scalar
                // pointer fields, we cast the ulong back to the original pointer
                // type. For array-of-pointer fields, we set the type to the
                // lowered array type (Array(UIntPtr, N)) and insert casts at each
                // GetElement (by-value array indexing) that extracts an element.
                for (auto inst : fieldExtractInsts)
                {
                    if (as<IRArrayTypeBase>(fieldInfo.originalPtrType))
                    {
                        inst->setFullType(loweredFieldType);
                        // Walk uses of the extracted array to find GetElement ops
                        // that extract individual ulong elements — these need a
                        // CastIntToPtr to restore the pointer type.
                        List<IRInst*> arrayUses;
                        for (auto use = inst->firstUse; use; use = use->nextUse)
                            arrayUses.add(use->getUser());
                        for (auto arrayUser : arrayUses)
                        {
                            if (arrayUser->getOp() == kIROp_GetElement)
                            {
                                arrayUser->setFullType(uintPtrType);
                                builder.setInsertAfter(arrayUser);
                                auto castInst =
                                    builder.emitCastIntToPtr(origElemPtrType, arrayUser);
                                arrayUser->replaceUsesWith(castInst);
                                castInst->setOperand(0, arrayUser);
                            }
                        }
                    }
                    else
                    {
                        // Scalar pointer field: cast ulong back to typed pointer.
                        inst->setFullType(uintPtrType);
                        builder.setInsertAfter(inst);
                        auto castInst = builder.emitCastIntToPtr(origElemPtrType, inst);
                        inst->replaceUsesWith(castInst);
                        castInst->setOperand(0, inst);
                    }
                }
            }
        }
    }

    // --- StorageBuffer element lowering ---

    // Handles RWStructuredBuffer<T*> where the element type itself is a pointer.
    // The buffer is already a device pointer, so any pointer element creates
    // pointer-to-pointer which Metal rejects.
    //
    // The two-level use walk (bufType->firstUse → bufVar->firstUse) works because:
    // - bufType->firstUse enumerates every user of the buffer TYPE, which includes
    //   the IRGlobalParam (or ParameterBlock field) that declares the buffer variable.
    // - bufVar->firstUse then enumerates every instruction that references that
    //   variable, which includes all structured buffer access ops.
    // - The default: break in the switch safely drops non-access-op users (e.g.
    //   decorations, type references, KernelContext field addresses).
    //
    // For ParameterBlock-wrapped buffers (ParameterBlock<struct { RWStructuredBuffer<T*> }>):
    // the buffer access ops appear on the LOADED buffer value, not directly on
    // the struct field. This path handles standalone buffer bindings; the struct
    // field path (collectStructsToLower + insertCastsForStructFields) handles
    // the ParameterBlock's element struct separately.
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

            // replaceOperand may return a different inst if dedup merges this
            // buffer type with an existing one. This is safe — we collected
            // uses BEFORE mutating, so elemAccessInsts refers to the original
            // users regardless of what happens to the type node.
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
                // Consume/Append are desugared by lowerAppendConsumeStructuredBuffers.
                // LoadStatus is HLSL-only ([require(hlsl)]) and cannot reach Metal.
                // No synthesized conversion functions are needed for the active cases:
                // - pointer↔ulong is a single CastIntToPtr/CastPtrToInt instruction.
                // - No field-by-field pack/unpack routines required (unlike struct/matrix/
                //   bool conversions that need multi-step pack/unpack).
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

    // This function handles ALL possible users of a rewritten field address
    // (or storage buffer element pointer). The handlers are exhaustive for
    // this pipeline stage because:
    //
    // - Load: the primary read pattern (FieldAddress → Load → use value)
    // - Store (as destination): the primary write pattern (value → Store → FieldAddress)
    // - Store (as value): field address used as a pointer value, e.g. via
    //   __getAddress. The address of a ulong field IS a valid device pointer;
    //   stored as-is without casting.
    // - GetElementPtr / GetOffsetPtr: array indexing or pointer arithmetic
    //   through the field address. For multi-dim arrays, intermediate result
    //   types collapse to Ptr(ulong) — correct because the C-like emitter
    //   folds chains into subscript expressions without inspecting
    //   intermediate pointer types.
    // - else (fallthrough): any other user is a pointer-value use (e.g. the
    //   address passed to a surviving helper function). This is safe because
    //   the address type (Ptr(ulong)) is a valid device pointer in Metal.
    //   Atomics on pointer fields are excluded by the type system (IAtomicable
    //   doesn't include pointer types). Debug assert guards against future
    //   address-deriving ops falling through unhandled.
    //
    // Ops that do not appear as users of a rewritten field address:
    // - Phi / block parameters: eliminated by eliminatePhis before this pass.
    // - CopyLogical: SPIR-V-specific workaround; irrelevant for Metal.
    // - Matrix element addressing: lowered before this pass; unrelated to
    //   pointer-to-pointer.
    void insertCastsForPointerUses(
        IRInst* startAddress,
        IRType* originalPtrType,
        IRType* uintPtrType)
    {
        List<IRInst*> worklist;
        worklist.add(startAddress);

        while (worklist.getCount() > 0)
        {
            auto addressInst = worklist.getLast();
            worklist.removeLast();

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
                    if (storeInst->getPtr() == addressInst)
                    {
                        // addressInst is the DESTINATION — cast the stored value
                        // from pointer to ulong.
                        auto val = storeInst->getVal();
                        builder.setInsertBefore(storeInst);
                        IRInst* args[] = {val};
                        auto castInst =
                            builder.emitIntrinsicInst(uintPtrType, kIROp_CastPtrToInt, 1, args);
                        storeInst->setOperand(1, castInst);
                    }
                    // else: addressInst is the VALUE being stored (e.g. via
                    // __getAddress). The address of a ulong field is a valid
                    // device pointer and can be stored as-is.
                }
                else if (user->getOp() == kIROp_GetElementPtr ||
                         user->getOp() == kIROp_GetOffsetPtr)
                {
                    // Array indexing or pointer arithmetic derives a new
                    // address from this one. Update its result type to
                    // Ptr(ulong) and add to worklist so its uses get patched.
                    auto ptrToElem = as<IRPtrTypeBase>(user->getFullType());
                    if (ptrToElem)
                        user->setFullType(builder.getPtrType(uintPtrType, ptrToElem));
                    worklist.add(user);
                }
                else
                {
                    // Any remaining user treats the address as an opaque
                    // pointer value (e.g. passed to a helper function).
                    // This is safe because Ptr(ulong) is a valid device
                    // pointer type in Metal — no cast needed on the address
                    // itself, only on values loaded from it.
                    SLANG_ASSERT(!isAddressInst(user));
                }
            }
        }
    }

    // A single collect-rewrite-cast traversal suffices:
    // - No fixpoint iteration needed.
    // - In-place type mutation is visible module-wide; no need to propagate
    //   rewritten types through call boundaries.
    // - All access sites directly reachable via key use-chains.
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
