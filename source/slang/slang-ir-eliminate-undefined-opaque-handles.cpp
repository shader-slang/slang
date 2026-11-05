// slang-ir-eliminate-undefined-opaque-handles.cpp
#include "slang-ir-eliminate-undefined-opaque-handles.h"

#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-legalize-types.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
// A `LoadFromUninitializedMemory` yields an arbitrary value of its type, and a pass may choose one
// particular value for all of its uses. For an opaque handle such as a `Texture2D`, SPIR-V for
// Vulkan has no undefined value at all: spirv-val rejects `OpUndef` of an image, sampler or
// acceleration-structure type (VUID-StandaloneSpirv-OpUndef-12464). We therefore choose, once for
// each undefined handle and so consistently for all of its uses, a handle to a descriptor that was
// never written. Accessing a resource through such a handle is undefined behavior on Vulkan, so
// each operation that does so may yield any result and have any effect, including none.
//
// Consider this example:
//
//     Texture2D<float4> t;
//     float4 r = 0;
//     if (c)
//         r = t.SampleLevel(s, uv, 0);
//
// By the time this pass runs, the sample is a `SPIRVAsm` block that reads
// `makeCombinedTextureSampler(t, s)`. Sampling through the combined texture-sampler touches the
// never-written descriptor, so we replace the result of the block with a fresh
// `LoadFromUninitializedMemory<float4>` and remove the block. That leaves the combined
// texture-sampler unused, and then `t`, and dead-code elimination removes them. Each result is a
// frozen value rather than `Poison`. A `Poison` handle is treated like a
// `LoadFromUninitializedMemory`, since poison may be refined to any value. Undefined values of
// other types are not touched.
//
// We rewrite only the uses that we recognize as accesses, or as deriving a value that leads to
// accesses, such as a combined texture-sampler or an address into the contents of the resource.
// Every other use, such as a call argument or a store of the handle itself, is left in place. A
// handle that such a use keeps alive is reported by `diagnoseUndefinedOpaqueHandles`, so a use that
// the recognizers below miss fails compilation rather than being rewritten wrongly.
//
// If `t` is instead passed to a function that samples it, we specialize the call: the callee is
// cloned with `t`'s parameter replaced by an undefined handle of its own, and the sample in the
// clone is then rewritten as above. Each undefined handle stands for the same never-written
// descriptor, so the clone needs no connection to the caller's `t`.
//
// The simplifier, which also runs on each clone that specialization creates, removes some uses
// before this pass sees them: it removes a `DebugValue` of an undefined value, and folds the
// texture or sampler extracted from a combined texture-sampler to that operand of it, or to an
// undefined value if the combined texture-sampler is itself undefined. We do not recognize those
// uses here, so one that reaches this pass anyway is reported.

/// Return whether `inst` is an undefined value, a `LoadFromUninitializedMemory` or a `Poison`, of
/// an image, sampler or acceleration-structure type. VUID-12464 forbids exactly these types as the
/// type of an `OpUndef`.
static bool isUndefinedOpaqueHandle(IRInst* inst)
{
    return as<IRUndefined>(inst) && isImageSamplerOrAccelerationStructureType(inst->getDataType());
}

/// Call `f` on every undefined opaque handle in `module`. The handles are collected first, so `f`
/// may remove the handle it is given.
template<typename F>
static void forEachUndefinedOpaqueHandle(IRModule* module, F const& f)
{
    List<IRInst*> handles;

    // `IRUndefined` has two kinds, `LoadFromUninitializedMemory` and `Poison`, and we look for
    // both. A `Poison` is hoistable, so it lives at module scope, where `overAllBlocks` does not
    // look.
    for (auto globalInst : module->getGlobalInsts())
    {
        if (isUndefinedOpaqueHandle(globalInst))
            handles.add(globalInst);
    }

    // A `LoadFromUninitializedMemory` lives in the function that reads the uninitialized memory.
    overAllBlocks(
        module,
        [&](IRBlock* block)
        {
            for (auto inst : block->getChildren())
            {
                if (isUndefinedOpaqueHandle(inst))
                    handles.add(inst);
            }
        });

    for (auto handle : handles)
        f(handle);
}

/// Return the image that `inst` reads or writes if `inst` is a dedicated image operation, or null
/// otherwise.
///
/// Sampling, queries and ray-tracing operations do not appear here: by the time this pass runs they
/// are `SPIRVAsm` blocks, which `rewriteUse` handles.
static IRInst* getAccessedImage(IRInst* inst)
{
    if (auto load = as<IRImageLoad>(inst))
        return load->getImage();
    if (auto store = as<IRImageStore>(inst))
        return store->getImage();
    if (auto gather = as<IRImageGatherOffset>(inst))
        return gather->getSampledImage();
    if (auto load = as<IRSubpassLoad>(inst))
        return load->getSubpassInput();
    return nullptr;
}

/// Return whether `use`, a use of a value on the work list, is the image operand of a dedicated
/// image operation.
static bool doesUseAccessImage(IRUse* use)
{
    auto image = getAccessedImage(use->getUser());
    if (!image)
        return false;

    // The other operands of these operations, such as a coordinate or a texel to store, are never
    // of an image type.
    SLANG_ASSERT(use->get() == image);
    return true;
}

/// Return whether `use`, a use of a value on the work list, is the image operand of an
/// `ImageSubscript`, which yields an address into the contents of that image.
static bool doesUseDeriveAddressIntoImage(IRUse* use)
{
    auto subscript = as<IRImageSubscript>(use->getUser());
    if (!subscript)
        return false;

    // The other operands of an `ImageSubscript` are coordinates.
    SLANG_ASSERT(use->get() == subscript->getImage());
    return true;
}

/// Return whether `use`, a use of a value on the work list, is the address that an atomic
/// operation reads and writes.
///
/// A load or a store through an address into an image is lowered to an image operation before
/// this pass runs, so only an atomic operation still uses such an address here.
static bool doesUseAccessAddress(IRUse* use)
{
    auto atomic = as<IRAtomicOperation>(use->getUser());
    if (!atomic)
        return false;

    // The other operands of an atomic operation are values and memory-order flags.
    SLANG_ASSERT(use->get() == atomic->getPtr());
    return true;
}

/// Return whether `combined` combines a texture and a sampler of which at least one is an
/// undefined opaque handle.
static bool doesCombineUndefinedHandle(IRMakeCombinedTextureSampler* combined)
{
    if (isUndefinedOpaqueHandle(combined->getTexture()))
        return true;
    if (isUndefinedOpaqueHandle(combined->getSampler()))
        return true;
    return false;
}

/// Specialize each call that passes an undefined opaque handle, or a combined texture-sampler that
/// includes one, so that the callee gets an undefined handle of its own whose accesses we can
/// rewrite.
struct UndefinedOpaqueHandleArgSpecializationCondition : FunctionCallSpecializeCondition
{
    typedef FunctionCallSpecializeCondition Super;

    bool doesParamWantSpecialization(IRParam* param, IRInst* arg, IRCall* call) override
    {
        SLANG_UNUSED(param);
        SLANG_UNUSED(call);
        if (isUndefinedOpaqueHandle(arg))
            return true;
        if (auto combined = as<IRMakeCombinedTextureSampler>(arg))
            return doesCombineUndefinedHandle(combined);
        return false;
    }

    bool isParamSuitableForSpecialization(IRParam* param, IRInst* arg) override
    {
        // The specialized callee gets a fresh undefined handle of the same type.
        if (isUndefinedOpaqueHandle(arg))
            return true;

        // The specialized callee rebuilds the combined texture-sampler, so each operand must be an
        // undefined handle or a value that the callee can refer to, such as a global parameter.
        if (auto combined = as<IRMakeCombinedTextureSampler>(arg))
        {
            if (!isOperandSuitableForSpecialization(param, combined->getTexture()))
                return false;
            if (!isOperandSuitableForSpecialization(param, combined->getSampler()))
                return false;
            return true;
        }

        return false;
    }

    /// Return whether `operand`, the texture or sampler of a combined texture-sampler passed for
    /// `param`, can be referred to from a specialized callee.
    bool isOperandSuitableForSpecialization(IRParam* param, IRInst* operand)
    {
        if (isUndefinedOpaqueHandle(operand))
            return true;
        return Super::isParamSuitableForSpecialization(param, operand);
    }
};

struct UndefinedOpaqueHandleEliminationContext
{
    IRModule* module;

    /// The values through which every access touches a never-written descriptor: the undefined
    /// handles, then the values derived from them that `rewriteUse` adds, namely combined
    /// texture-samplers that include one and addresses into the contents of an image that one
    /// refers to.
    InstWorkList workList;

    /// The values in `workList`. A combined texture-sampler whose texture and sampler are both
    /// undefined is reached from each of them, and is listed once.
    InstHashSet workListSet;

    UndefinedOpaqueHandleEliminationContext(IRModule* module)
        : module(module), workList(module), workListSet(module)
    {
    }

    void addToWorkList(IRInst* value)
    {
        if (workListSet.add(value))
            workList.add(value);
    }

    /// Rewrite the uses of each value in the work list, including the values that are added while
    /// we go.
    ///
    /// A value left without uses is removed by the dead-code elimination that `legalizeIRForSPIRV`
    /// runs before `diagnoseUndefinedOpaqueHandles`, which then reports each undefined handle that
    /// a use left in place keeps alive.
    void processWorkList()
    {
        // `traverseUses` lists the uses of a value before visiting them, and skips a use that an
        // earlier visit has removed. `t.GetDimensions(0, w, h, levels)` becomes a `SPIRVAsm` block
        // that names `t` once for the size and once for the number of levels; replacing the block
        // at the first use removes the second.
        for (Index i = 0; i < workList.getCount(); i++)
            traverseUses(workList[i], [&](IRUse* use) { rewriteUse(use); });
    }

    /// Rewrite `use`, a use of a value on the work list, if it accesses a resource through that
    /// value, or add the value it derives to the work list if accesses can happen through that.
    /// Leave every other use in place.
    void rewriteUse(IRUse* use)
    {
        auto user = use->getUser();

        // We cannot see what a `SPIRVAsm` block does with its operands, so we treat a block that
        // names the value as accessing the resource. The blocks that this pass meets come from
        // intrinsic functions that read, write or query a resource, and for those this is exact.
        if (auto asmOperand = as<IRSPIRVAsmOperand>(user))
        {
            replaceWithUndefinedResult(asmOperand->getAsmBlock());
            return;
        }

        // A combined texture-sampler is not an access, and may hold a defined texture or sampler
        // that something extracts again. An operation through it, such as a sample, touches the
        // never-written descriptor, just as an operation through the undefined handle would.
        if (auto combined = as<IRMakeCombinedTextureSampler>(user))
        {
            addToWorkList(combined);
            return;
        }

        // We rewrite the uses of an address into the image rather than replace the address with an
        // undefined value, because an atomic operation through an undefined pointer would still
        // have to be removed, and SPIR-V for Vulkan has no valid undefined pointer to dereference.
        if (doesUseDeriveAddressIntoImage(use))
        {
            addToWorkList(user);
            return;
        }

        if (doesUseAccessImage(use) || doesUseAccessAddress(use))
            replaceWithUndefinedResult(user);
    }

    /// Replace the result of `operation`, which accesses a resource through an undefined handle,
    /// by a fresh undefined value of the same type, and remove `operation`, which removes its
    /// effect too.
    void replaceWithUndefinedResult(IRInst* operation)
    {
        if (operation->hasUses())
        {
            // An access that yields no value, such as an image store, has no uses.
            auto type = operation->getDataType();
            SLANG_ASSERT(type && !as<IRVoidType>(type));

            IRBuilder builder(module);
            builder.setInsertBefore(operation);
            auto undefinedResult =
                builder.emitLoadFromUninitializedMemory(operation->getFullType());
            operation->replaceUsesWith(undefinedResult);
        }
        operation->removeAndDeallocate();
    }
};

void eliminateUndefinedOpaqueHandles(IRModule* module, CodeGenContext* codeGenContext)
{
    // A module without an undefined opaque handle has no call to specialize and no access to
    // rewrite, so we skip the walk over every call that `specializeFunctionCalls` makes.
    bool hasUndefinedHandle = false;
    forEachUndefinedOpaqueHandle(module, [&](IRInst*) { hasUndefinedHandle = true; });
    if (!hasUndefinedHandle)
        return;

    UndefinedOpaqueHandleArgSpecializationCondition condition;
    UndefinedOpaqueHandleEliminationContext context(module);

    // We first specialize each call that passes an undefined handle, or a combined texture-sampler
    // that includes one, until no such call that we can specialize is left, as
    // `specializeResourceParameters` does. The specializer also examines the calls in each clone
    // it creates, after simplifying the clone, and the simplifier folds the texture or sampler
    // extracted from an undefined combined texture-sampler to an undefined handle. A call in a
    // clone that passes such a handle is therefore specialized as well.
    //
    // Specialization ends, because each clone has fewer image, sampler or acceleration-structure
    // parameters than the function it was cloned from: specializing a call removes at least one
    // and adds none. The number of clones can still grow with each distinct set of arguments, as it
    // can for global resources: a call tree in which each function calls the next twice, passing a
    // different subset of undefined handles, can need a clone for every subset.
    while (specializeFunctionCalls(codeGenContext, module, &condition))
    {
    }

    // After specialization, no call that we could specialize passes an undefined handle, so we can
    // rewrite the accesses through each one. A call that we could not specialize, such as one to a
    // function without a body, still passes its handle, which is then reported.
    forEachUndefinedOpaqueHandle(module, [&](IRInst* handle) { context.addToWorkList(handle); });
    context.processWorkList();
}

void diagnoseUndefinedOpaqueHandles(IRModule* module, DiagnosticSink* sink)
{
    forEachUndefinedOpaqueHandle(
        module,
        [&](IRInst* handle)
        {
            sink->diagnose(Diagnostics::UseOfUninitializedOpaqueHandle{
                .handleType = handle->getDataType(),
                .location = findBestSourceLocFromUses(handle)});
        });
}
} // namespace Slang
