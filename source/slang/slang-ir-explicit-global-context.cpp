// slang-ir-explicit-global-context.cpp
#include "slang-ir-explicit-global-context.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-util.h"

namespace Slang
{

// The job of this pass is take global-scope declarations
// that are actually scoped to a single shader thread or
// thread-group, and wrap them up in an explicit "context"
// type that gets passed between functions.
//
// WGSL can keep thread-local globals as `var<private>`, but its uniformity analysis treats
// every read of one as possibly non-uniform. On WGSL the pass therefore moves the shadow copies
// of workgroup-uniform built-in inputs, so that subgroup operations and barriers whose control
// flow depends on those values still validate. It moves only those: the analysis does not track
// struct fields separately, so a single non-uniform field, or a store under divergent control
// flow, would make every read through the context non-uniform.

// Return true when `user`, an instruction that uses an address, is known to only read the memory
// at that address, either directly or through element and field addresses derived from it. Any
// other use returns false, so a caller that needs a proof of read-only access stays conservative.
static bool isReadOnlyAddressUse(IRInst* user)
{
    switch (user->getOp())
    {
    case kIROp_Load:
        return true;
    case kIROp_GetElementPtr:
    case kIROp_FieldAddress:
        for (auto use = user->firstUse; use; use = use->nextUse)
        {
            if (!isReadOnlyAddressUse(use->getUser()))
                return false;
        }
        return true;
    default:
        return false;
    }
}

// Return whether `param`, the parameter that carries a
// `GlobalVariableShadowingGlobalParameterDecoration`, belongs to a compute entry point and has a
// system-value semantic whose WGSL built-in is workgroup-uniform. A copy of an entry point that
// `fixEntryPointCallsites` makes for direct calls keeps the decoration but not the entry-point
// decoration, so it is rejected here.
static bool isWorkgroupUniformEntryPointInput(IRInst* param)
{
    auto func = getParentFunc(param);
    auto entryPointDecor = func ? func->findDecoration<IREntryPointDecoration>() : nullptr;
    if (!entryPointDecor)
        return false;

    // `translateGlobalVaryingVar` gives every parameter it creates for a global input a layout,
    // and WGSL entry-point legalization rewrites such a parameter's type in place, keeping the
    // parameter, its layout and the decoration.
    auto varLayout = findVarLayout(param);
    SLANG_RELEASE_ASSERT(varLayout);
    auto semanticAttr = varLayout->findSystemValueSemanticAttr();
    if (!semanticAttr)
        return false;
    return isWGSLWorkgroupUniformSystemValue(
        convertSystemValueSemanticNameToEnum(String(semanticAttr->getName())),
        entryPointDecor->getProfile().getStage());
}

// Return whether `globalVar` is a shadow copy that `translateGlobalVaryingVar` made of a
// workgroup-uniform built-in input, and whether its value stays uniform.
//
// Consider this example:
//
//     in uint gLaneCount : SV_WaveLaneCount;
//     [noinline] uint numWaves() { return 256 / gLaneCount; }
//
// `translateGlobalVaryingVar` gives the entry point a `gLaneCount` parameter, stores that
// parameter into the global at the top of the entry block, and links the two with a
// `GlobalVariableShadowingGlobalParameterDecoration` on the parameter. The global qualifies when
// at least one such link exists, every linked parameter belongs to a compute entry point and has
// a workgroup-uniform semantic, and the only stores are those initializations. A shader can
// also write to a global input (`gLaneCount = 3;`), and such a write could make the value
// non-uniform, so any other store, or any use that lets the address escape, disqualifies it.
//
// `translateGlobalVaryingVar` emits exactly one initialization per linked entry point, at the top
// of its first block, so equal counts with every store in a linked entry block identify those
// initializations without matching their values.
static bool isWorkgroupUniformInputShadow(IRInst* globalVar)
{
    List<IRFunc*> shadowingEntryPoints;
    List<IRStore*> stores;
    for (auto use = globalVar->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (user->getOp() == kIROp_GlobalVariableShadowingGlobalParameterDecoration)
        {
            auto param = user->getParent();
            if (!isWorkgroupUniformEntryPointInput(param))
                return false;
            shadowingEntryPoints.add(getParentFunc(param));
            continue;
        }
        if (as<IRDecoration>(user))
            continue;
        if (auto store = as<IRStore>(user))
        {
            if (use != store->getPtrUse())
                return false;
            stores.add(store);
            continue;
        }
        if (!isReadOnlyAddressUse(user))
            return false;
    }

    if (shadowingEntryPoints.getCount() == 0 ||
        stores.getCount() != shadowingEntryPoints.getCount())
        return false;
    for (auto store : stores)
    {
        auto func = getParentFunc(store);
        if (!shadowingEntryPoints.contains(func) || store->getParent() != func->getFirstBlock())
            return false;
    }
    return true;
}

enum class GlobalObjectKind : UInt
{
    None = 0,
    GlobalVar = 1 << 0,
    GlobalParam = 1 << 1,
    All = 0xFFFFFFFF,
};

enum class HoistGlobalVarOptions : UInt
{
    PlainGlobal = 0,
    SharedGlobal = 1 << 0,
    RaytracingGlobal = 1 << 1,
    All = 0xFFFFFFFF,
};

struct IntroduceExplicitGlobalContextPass
{

    // TODO: (#4742) Discontinuity of AddressSpace values between targets
    // (SpvStorageClassFunction vs. AddressSpace::ThreadLocal) needs
    // to be addressed. This means `addressSpaceOfLocals` may be refactored out.

    /// Target specific options to manage `IntroduceExplicitGlobalContextPass`
    class ExplicitContextPolicy
    {
    public:
        ExplicitContextPolicy(CodeGenTarget inTarget)
            : target(inTarget)
        {
            switch (target)
            {
            case CodeGenTarget::SPIRV:
            case CodeGenTarget::SPIRVAssembly:
                hoistableGlobalObjectKind = GlobalObjectKind::GlobalVar;
                requiresFuncTypeCorrectionPass = true;
                addressSpaceOfLocals = AddressSpace::Function;
                hoistGlobalVarOptions = HoistGlobalVarOptions::PlainGlobal;
                break;
            case CodeGenTarget::WGSL:
                // WGSL keeps every thread-local global as a `var<private>` except the shadows of
                // workgroup-uniform built-in inputs, which the filter below selects; resource
                // bindings and `var<workgroup>` never pass it and stay at module scope. A single
                // such value reaches the functions that read it as an ordinary parameter, so a
                // function that needs one built-in takes just that value rather than a pointer to
                // a one-field struct; several are combined in one context. Locals and the context
                // pointer use the `function` address space, which WGSL requires for a pointer
                // parameter to a function-local variable.
                hoistableGlobalObjectKind = GlobalObjectKind::GlobalVar;
                hoistGlobalVarOptions = HoistGlobalVarOptions::PlainGlobal;
                addressSpaceOfLocals = AddressSpace::Function;
                hoistOnlyWorkgroupUniformInputShadows = true;
                passSoleValueByValue = true;
                break;
            case CodeGenTarget::CUDASource:
            case CodeGenTarget::CUDAHeader:
                hoistableGlobalObjectKind = GlobalObjectKind::GlobalVar;

                // One important exception is that CUDA *does* support
                // global variables with the `__shared__` qualifer, with
                // semantics that exactly match HLSL/Slang `groupshared`.
                //
                // We thus need to skip processing of global variables
                // that were marked `groupshared`. In our current IR,
                // this is represented as a variable with the `@GroupShared`
                // rate on its type.
                //
                hoistGlobalVarOptions = HoistGlobalVarOptions(
                    0 | (UInt)HoistGlobalVarOptions::PlainGlobal |
                    (UInt)HoistGlobalVarOptions::RaytracingGlobal);
                break;
            }

            // A global passed by value loses writes made outside the entry point, so that option
            // is only combined with the filter that admits globals no other function writes. It
            // also makes every function's context a local `Var`, which the function-type fix-up
            // in `processModule` takes to mean an entry point.
            SLANG_ASSERT(!passSoleValueByValue || hoistOnlyWorkgroupUniformInputShadows);
            SLANG_ASSERT(!passSoleValueByValue || !requiresFuncTypeCorrectionPass);
        }

        bool canHoistType(GlobalObjectKind hoistable)
        {
            return (UInt)hoistableGlobalObjectKind & (UInt)hoistable;
        }

        bool canHoistGlobalVar(IRInst* inst)
        {
            if (hoistOnlyWorkgroupUniformInputShadows && !isWorkgroupUniformInputShadow(inst))
                return false;

            if (!((UInt)hoistGlobalVarOptions & (UInt)HoistGlobalVarOptions::SharedGlobal) &&
                as<IRGroupSharedRate>(inst->getRate()))
                return false;

            if (!((UInt)hoistGlobalVarOptions & (UInt)HoistGlobalVarOptions::RaytracingGlobal))
            {
                for (auto decoration : inst->getDecorations())
                {
                    switch (decoration->getOp())
                    {
                    case kIROp_VulkanRayPayloadDecoration:
                    case kIROp_VulkanRayPayloadInDecoration:
                    case kIROp_VulkanCallablePayloadDecoration:
                    case kIROp_VulkanCallablePayloadInDecoration:
                    case kIROp_VulkanHitObjectAttributesDecoration:
                    case kIROp_VulkanHitAttributesDecoration:
                        return false;
                    default:
                        continue;
                    };
                }
            }

            // Do not move specialization constants to context.
            switch (target)
            {
            case CodeGenTarget::Metal:
            case CodeGenTarget::MetalLib:
            case CodeGenTarget::MetalLibAssembly:
                {
                    auto varLayout = findVarLayout(inst);
                    if (varLayout &&
                        varLayout->findOffsetAttr(LayoutResourceKind::SpecializationConstant))
                        return false;
                }
            }
            return true;
        }

        bool requiresFuncTypeCorrection() { return requiresFuncTypeCorrectionPass; }

        AddressSpace getAddressSpaceOfLocal() { return addressSpaceOfLocals; }

        bool shouldPassSoleValueByValue() { return passSoleValueByValue; }

    private:
        HoistGlobalVarOptions hoistGlobalVarOptions = HoistGlobalVarOptions::All;
        GlobalObjectKind hoistableGlobalObjectKind = GlobalObjectKind::All;
        bool requiresFuncTypeCorrectionPass = false;
        bool hoistOnlyWorkgroupUniformInputShadows = false;

        // When exactly one global is hoisted, the context is that global's value itself rather
        // than a struct, and functions receive it by value. This is only correct when no
        // function other than an entry point writes the global, so a policy that sets it must
        // hoist only such globals.
        bool passSoleValueByValue = false;
        AddressSpace addressSpaceOfLocals = AddressSpace::ThreadLocal;
        CodeGenTarget target;
    };

    IntroduceExplicitGlobalContextPass(IRModule* module, CodeGenTarget target)
        : m_module(module), m_target(target), m_options(target)
    {
    }

    IRModule* m_module = nullptr;
    CodeGenTarget m_target = CodeGenTarget::Unknown;

    // The context is usually a `KernelContext` struct passed by pointer. When the policy passes
    // a sole hoisted global by value, `m_contextType` is instead that global's value type, and
    // `m_contextStructType` and `m_contextPtrType` stay null.
    IRStructType* m_contextStructType = nullptr;
    IRType* m_contextType = nullptr;
    IRPtrType* m_contextPtrType = nullptr;
    bool m_passContextByValue = false;

    struct GlobalParamInfo
    {
        // Original global param inst.
        IRGlobalParam* globalParam = nullptr;

        // New entry point param that is created by this pass.
        IRParam* entryPointParam = nullptr;

        // Orignating entry point obtained from entry point param decoration, if it exists.
        IRFunc* originatingEntryPoint = nullptr;
    };

    List<GlobalParamInfo> m_globalParams;
    List<IRGlobalVar*> m_globalVars;
    List<IRFunc*> m_entryPoints;

    ExplicitContextPolicy m_options;

    AddressSpace getAddressSpaceOfLocal() { return m_options.getAddressSpaceOfLocal(); }

    bool canHoistType(GlobalObjectKind hoistable) { return m_options.canHoistType(hoistable); }

    bool canHoistGlobalVar(IRInst* inst) { return m_options.canHoistGlobalVar(inst); }

    void processModule()
    {
        IRBuilder builder(m_module);

        // The transformation we will perform will need to affect
        // global variables, global shader parameters, and entry-point
        // function (at the very least), and we start with an explicit
        // pass to collect these entities into explicit lists to simplify
        // looping over them later.
        //
        for (auto inst : m_module->getGlobalInsts())
        {
            switch (inst->getOp())
            {
            case kIROp_GlobalVar:
                {
                    if (!canHoistType(GlobalObjectKind::GlobalVar))
                        continue;
                    // A "global variable" in HLSL (and thus Slang) is actually
                    // a weird kind of thread-local variable, and so it cannot
                    // actually be lowered to a global variable on targets where
                    // globals behave like, well, globals.
                    //
                    auto globalVar = cast<IRGlobalVar>(inst);

                    // Actual globals don't need to be moved to the context
                    if (as<IRActualGlobalRate>(globalVar->getRate()))
                    {
                        continue;
                    }

                    if (!canHoistGlobalVar(globalVar))
                        continue;

                    m_globalVars.add(globalVar);
                }
                break;

            case kIROp_GlobalParam:
                {
                    if (!canHoistType(GlobalObjectKind::GlobalParam))
                        continue;
                    // Global parameters are another HLSL/Slang concept
                    // that doesn't have a parallel in languages like C/C++.
                    //
                    auto globalParam = cast<IRGlobalParam>(inst);

                    if (!canHoistGlobalVar(globalParam))
                        continue;

                    // One detail we need to be careful about is that as a result
                    // of legalizing the varying parameters of compute kernels to
                    // CPU or CUDA, we can end up with global parameters for varying
                    // parameters on CUDA (e.g., to represent `threadIdx`. We thus
                    // skip any global-scope parameters that are varying instead of
                    // uniform.
                    //
                    if (isCUDATarget(m_target) || isCPUTarget(m_target))
                    {
                        auto layoutDecor = globalParam->findDecoration<IRLayoutDecoration>();
                        SLANG_ASSERT(layoutDecor);
                        auto layout = as<IRVarLayout>(layoutDecor->getLayout());
                        SLANG_ASSERT(layout);
                        if (isVaryingParameter(layout))
                            continue;
                    }

                    // Because of upstream passes, we expect there to be only a
                    // single global uniform parameter (at most).
                    //
                    // Note: If we ever changed out mind about the representation
                    // and wanted to support multiple global parameters, we could
                    // easily generalize this code to work with a list.

                    // For CUDA output, we want to leave the global uniform
                    // parameter where it is, because it will translate to
                    // a global `__constant__` variable.
                    if (m_target == CodeGenTarget::CUDASource ||
                        m_target == CodeGenTarget::CUDAHeader)
                        continue;

                    GlobalParamInfo globalParamInfo;
                    globalParamInfo.globalParam = globalParam;

                    // Entry point param decorations are not required anymore after this pass and
                    // must be removed for entry point param emit. Remoeving it here prevents the
                    // decoration from being cloned when creating struct keys and entry point
                    // parameters.
                    if (const auto entryPointParamDecoration =
                            globalParam->findDecoration<IREntryPointParamDecoration>())
                    {
                        globalParamInfo.originatingEntryPoint =
                            entryPointParamDecoration->getEntryPoint();
                        entryPointParamDecoration->removeAndDeallocate();
                    }

                    m_globalParams.add(globalParamInfo);
                }
                break;

            case kIROp_Func:
                {
                    // Every entry point function is going to need to be modified,
                    // so that it can explicit create the context that other
                    // operations will use.

                    // We need to filter the IR functions to find only those
                    // that represent entry points.
                    //
                    auto func = cast<IRFunc>(inst);
                    // Each shader or CUDA launch creates per-invocation global storage. Ordinary
                    // clones instead receive their caller's context through the direct call.
                    if (!isShaderOrCudaKernelEntryPoint(func))
                        continue;

                    m_entryPoints.add(func);
                }
                break;
            }
        }

        // If there are no global-scope entities that require processing,
        // then we can completely skip the work of this pass for CUDA/Metal.
        //
        // Note: We cannot skip the rest of the pass for CPU, because
        // it is responsible for introducing the explicit entry-point
        // parameter that is used for passing in the global param(s).
        //
        if (!isCPUTarget(m_target))
        {
            if (m_globalParams.getCount() == 0 && m_globalVars.getCount() == 0)
            {
                return;
            }
        }

        // Now that we've capture all the relevant global entities from the IR,
        // we can being to transform them in an appropriate order.
        //
        m_passContextByValue = m_options.shouldPassSoleValueByValue() &&
                               m_globalParams.getCount() == 0 && m_globalVars.getCount() == 1;
        if (m_passContextByValue)
        {
            // A sole hoisted global is its own context: the value stands in for the global in
            // each function, so no field key is needed to reach it.
            //
            auto globalVar = m_globalVars[0];
            m_contextType = globalVar->getDataType()->getValueType();
            m_mapInstToContextFieldInfo.add(globalVar, ContextFieldInfo{});
        }
        else
        {
            createContextStructType(builder);
        }

        // Once the context type is known, we can process the entry points.
        //
        // Each entry point will create a local context variable and
        // initialize it based on the parameters passed to the entry point.
        //
        // The local variable introduced here will be registered as the representation
        // of the context to be used in the body of the entry point.
        //
        for (auto entryPoint : m_entryPoints)
        {
            createContextForEntryPoint(entryPoint);
        }

        // Now that we've prepared all the entry points, we can make another
        // pass over the global parameters/variables and start to replace
        // their use sites with references to the fields of the context.
        //
        // Wherever a global parameter/variable is being referenced in a function,
        // we will need to find or create a context value for that function
        // to use. The context value for entry points has already been established
        // above, but other functions will have an explicit context parameter
        // added on demand.
        //
        for (auto globalParam : m_globalParams)
        {
            replaceUsesOfGlobalParam(globalParam.globalParam);
        }
        for (auto globalVar : m_globalVars)
        {
            replaceUsesOfGlobalVar(globalVar);
        }

        // SPIRV requires a correct IR func-type to emit properly
        if (m_options.requiresFuncTypeCorrection())
        {
            for (auto pairOfFuncs : m_mapFuncToContextPtr)
            {
                if (pairOfFuncs.second->getOp() == kIROp_Var)
                    continue;
                fixUpFuncType(pairOfFuncs.first);
            }
        }
    }

    // Create the `KernelContext` struct and the pointer type it is passed by, add a field for
    // every hoisted global parameter and global variable, and record each field's key in
    // `m_mapInstToContextFieldInfo` for the later rewrites of their uses.
    //
    void createContextStructType(IRBuilder& builder)
    {
        m_contextStructType = builder.createStructType();
        m_contextType = m_contextStructType;
        if (m_target == CodeGenTarget::CPPHeader)
        {
            builder.addExternCppDecoration(
                m_contextStructType,
                UnownedTerminatedStringSlice("KernelContext"));
        }
        builder.addNameHintDecoration(
            m_contextStructType,
            UnownedTerminatedStringSlice("KernelContext"));

        // The struct is passed around by pointer, so we get and
        // cache that pointer type up front.
        //
        m_contextPtrType = builder.getPtrType(m_contextStructType, getAddressSpaceOfLocal());

        // The first step will be to create fields in the `KernelContext`
        // type to represent any global parameters or global variables.
        //
        // The keys for the fields that are created will be remembered
        // in a dictionary, so that we can find them later based on
        // the global parameter/variable.
        //
        for (auto globalParam : m_globalParams)
        {
            // For the parameter representing all the global uniform shader
            // parameters, we create a field that exactly matches its type.
            //
            createContextStructField(
                globalParam.globalParam,
                GlobalObjectKind::GlobalParam,
                globalParam.globalParam->getFullType());
        }
        for (auto globalVar : m_globalVars)
        {
            // A `IRGlobalVar` represents a pointer to where the variable is stored,
            // so we need to create a field of the pointed-to type to represent it.
            //
            createContextStructField(
                globalVar,
                GlobalObjectKind::GlobalVar,
                getGlobalVarPtrType(globalVar));
        }
    }

    // As noted above, we will maintain mappings to record
    // the key for the context field created for a global
    // variable parameter, and to record the context pointer
    // value to use for a function.
    //
    struct ContextFieldInfo
    {
        IRStructKey* key = nullptr;

        // Is this field a pointer to the actual value?
        // For groupshared variables, this will be true.
        bool needDereference = false;
    };
    Dictionary<IRInst*, ContextFieldInfo> m_mapInstToContextFieldInfo;
    Dictionary<IRFunc*, IRInst*> m_mapFuncToContextPtr;

    void createContextStructField(IRInst* originalInst, GlobalObjectKind kind, IRType* type)
    {
        // Creating a field in the context struct to represent
        // `originalInst` is straightforward.

        IRBuilder builder(m_module);
        builder.setInsertBefore(m_contextStructType);

        IRType* fieldDataType = type;
        bool needDereference = false;
        if (kind == GlobalObjectKind::GlobalVar)
        {
            auto ptrType = as<IRPtrTypeBase>(type);
            if (ptrType->getAddressSpace() == AddressSpace::GroupShared)
            {
                fieldDataType = ptrType;
                needDereference = true;
            }
            else
            {
                fieldDataType = as<IRPtrTypeBase>(type)->getValueType();
            }
        }

        // We create a "key" for the new field, and then a field
        // of the appropraite type.
        //
        auto key = builder.createStructKey();
        builder.createStructField(m_contextStructType, key, fieldDataType);

        key->sourceLoc = originalInst->sourceLoc;

        // Clone all original decorations to the new struct key.
        IRCloneEnv cloneEnv;
        cloneInstDecorationsAndChildren(&cloneEnv, m_module, originalInst, key);
        if (m_target == CodeGenTarget::CPPHeader)
        {
            builder.addExternCppDecoration(
                key,
                originalInst->findDecoration<IRNameHintDecoration>()->getName());
        }

        // We end by making note of the key that was created
        // for the instruction, so that we can use the key
        // to access the field later.
        //
        m_mapInstToContextFieldInfo.add(originalInst, ContextFieldInfo{key, needDereference});
    }

    void createContextForEntryPoint(IRFunc* entryPointFunc)
    {
        // We can only introduce the explicit context into
        // entry points that have definitions.
        //
        auto firstBlock = entryPointFunc->getFirstBlock();
        if (!firstBlock)
            return;

        IRBuilder builder(m_module);

        // The code we introduce will all be added to the start
        // of the first block of the function.
        //
        auto firstOrdinary = firstBlock->getFirstOrdinaryInst();
        builder.setInsertBefore(firstOrdinary);

        // If there was a global-scope uniform parameter before,
        // then we need to introduce an explicit parameter onto
        // each entry-point function to represent it.
        //

        List<GlobalParamInfo> entryPointParamsToAdd;
        for (auto globalParam : m_globalParams)
        {
            // Do not add global param to current entry point if global param
            // explicitly originates from a different entry point.
            if (globalParam.originatingEntryPoint &&
                globalParam.originatingEntryPoint != entryPointFunc)
            {
                continue;
            }

            globalParam.entryPointParam =
                builder.createParam(globalParam.globalParam->getFullType());
            IRCloneEnv cloneEnv;
            cloneInstDecorationsAndChildren(
                &cloneEnv,
                m_module,
                globalParam.globalParam,
                globalParam.entryPointParam);
            entryPointParamsToAdd.add(globalParam);

            // The new parameter will be the last one in the
            // parameter list of the entry point.
            //
            globalParam.entryPointParam->insertBefore(firstOrdinary);
        }

        if (isCPUTarget(m_target) && m_globalParams.getCount() == 0)
        {
            // The nature of our current ABI for entry points on CPU
            // means that we need an explicit parameter to be *declared*
            // for the global uniforms, even if it is never used.
            //
            auto placeholderParam = builder.createParam(builder.getRawPointerType());
            placeholderParam->insertBefore(firstOrdinary);
        }

        // The context to use inside the entry point (a `KernelContext`,
        // or the sole hoisted global's value) will be a local variable
        // declared in the first block.
        //
        auto contextVarPtr = builder.emitVar(m_contextType);
        addKernelContextNameHint(contextVarPtr);
        m_mapFuncToContextPtr.add(entryPointFunc, contextVarPtr);

        // If there is a global-scope uniform parameter, then
        // we need to use our new explicit entry point parameter
        // to inialize the corresponding field of the `KernelContext`
        // before moving on with execution of the kernel body.
        //
        for (auto entryPointParam : entryPointParamsToAdd)
        {
            auto fieldInfo = m_mapInstToContextFieldInfo[entryPointParam.globalParam];
            auto fieldType = entryPointParam.globalParam->getFullType();
            auto fieldPtrType = builder.getPtrType(fieldType);

            // We compute the addrress of the field and store the
            // value of the parameter into it.
            //
            auto fieldPtr = builder.emitFieldAddress(fieldPtrType, contextVarPtr, fieldInfo.key);
            builder.emitStore(fieldPtr, entryPointParam.entryPointParam);
        }

        // Note: at this point the `KernelContext` has additional
        // fields for global variables that do not seem to have
        // been initialized.
        //
        // Instead of making this pass take responsibility for initializing
        // global variables, it is instead expected that clients will
        // run the pass in `slang-ir-explicit-global-init` first,
        // in order to move all initialization of globals into the
        // entry point functions.
        //
        // To support groupshared variables on Metal,we need to allocate the
        // memory by defining a local variable in the entry point, and pass
        // the address of that variable to the context.
        //
        for (auto globalVar : m_globalVars)
        {
            auto fieldInfo = m_mapInstToContextFieldInfo[globalVar];
            if (fieldInfo.needDereference)
            {
                auto var = builder.emitVar(
                    globalVar->getDataType()->getValueType(),
                    AddressSpace::GroupShared);
                if (auto nameDecor = globalVar->findDecoration<IRNameHintDecoration>())
                {
                    builder.addNameHintDecoration(var, nameDecor->getName());
                }
                auto ptrPtrType =
                    builder.getPtrType(getGlobalVarPtrType(globalVar), getAddressSpaceOfLocal());
                auto fieldPtr = builder.emitFieldAddress(ptrPtrType, contextVarPtr, fieldInfo.key);
                builder.emitStore(fieldPtr, var);
            }
        }

        // Update entry point function type after potentially adding parameters.
        fixUpFuncType(entryPointFunc);
    }

    void replaceUsesOfGlobalParam(IRGlobalParam* globalParam)
    {
        IRBuilder builder(m_module);

        // A global shader parameter was mapped to a field
        // in the context structure, so we find the appropriate key.
        //
        auto fieldInfo = m_mapInstToContextFieldInfo[globalParam];

        auto valType = globalParam->getFullType();
        auto ptrType = builder.getPtrType(valType);

        // We then iterate over the uses of the parameter,
        // being careful to defend against the use/def information
        // being changed while we walk it.
        //
        IRUse* nextUse = nullptr;
        for (IRUse* use = globalParam->firstUse; use; use = nextUse)
        {
            nextUse = use->nextUse;

            // Consider this example:
            //
            //     AppendStructuredBuffer<int> output;
            //
            // Earlier passes split `output` into `output.elements` and `output.counter`, then
            // produce IR like this:
            //
            //     [CounterBuffer(%outputCounter)]
            //     %outputElements = global_param
            //     %outputCounter = global_param
            //
            // `CounterBuffer` identifies the counter associated with the elements parameter; it
            // does not read the counter's runtime value. This pass moves both parameters into a
            // structure:
            //
            //     %outputElementsKey = key
            //     %outputCounterKey = key
            //     struct KernelContext
            //     {
            //         field(%outputElementsKey, ...)
            //         field(%outputCounterKey, ...)
            //     }
            //
            // Therefore the decoration must become `CounterBuffer(%outputCounterKey)`. When this
            // loop processes `%outputCounter`, `fieldInfo.key` is `%outputCounterKey`, so replace
            // the decoration operand directly. Ordinary uses inside functions are instead
            // replaced with loads from the corresponding field below.
            auto user = use->getUser();
            if (as<IRDecoration>(user))
            {
                use->set(fieldInfo.key);
                continue;
            }

            // At each use site, we need to look up the context
            // pointer that is appropriate for that use.
            //
            auto contextParam = findOrCreateContextPtrForInst(user);
            builder.setInsertBefore(user);

            // The value of the parameter can be produced by
            // taking the address of the corresponding field
            // in the context struct and loading from it.
            //
            auto ptr = builder.emitFieldAddress(ptrType, contextParam, fieldInfo.key);
            auto val = builder.emitLoad(valType, ptr);
            use->set(val);
        }
    }

    IRType* getGlobalVarPtrType(IRGlobalVar* globalVar)
    {
        IRBuilder builder(globalVar);
        if (as<IRGroupSharedRate>(globalVar->getRate()))
        {
            return builder.getPtrType(
                globalVar->getDataType()->getValueType(),
                AddressSpace::GroupShared);
        }
        return builder.getPtrType(
            globalVar->getDataType()->getValueType(),
            getAddressSpaceOfLocal());
    }

    void replaceUsesOfGlobalVar(IRGlobalVar* globalVar)
    {
        IRBuilder builder(m_module);

        // A global variable was mapped to a field
        // in the context structure, so we find the appropriate key.
        //
        auto fieldInfo = m_mapInstToContextFieldInfo[globalVar];

        auto ptrType = getGlobalVarPtrType(globalVar);
        if (fieldInfo.needDereference)
            ptrType = builder.getPtrType(ptrType, getAddressSpaceOfLocal());

        // We then iterate over the uses of the variable,
        // being careful to defend against the use/def information
        // being changed while we walk it.
        //
        IRUse* nextUse = nullptr;
        for (IRUse* use = globalVar->firstUse; use; use = nextUse)
        {
            nextUse = use->nextUse;
            auto user = use->getUser();

            // Ensure the use site checked actually requires a replacement
            if (as<IRDecoration>(user))
                continue;

            // At each use site, we need to look up the context
            // pointer that is appropriate for that use.
            //
            auto contextPtr = findOrCreateContextPtrForInst(user);
            builder.setInsertBefore(user);

            // The address of the variable can be produced by
            // taking the address of the corresponding field
            // in the context struct. When a sole global is passed
            // by value, the function's local copy of that value
            // already stands in for the global.
            //
            IRInst* ptr = contextPtr;
            if (!m_passContextByValue)
                ptr = builder.emitFieldAddress(ptrType, contextPtr, fieldInfo.key);
            if (fieldInfo.needDereference)
                ptr = builder.emitLoad(ptr);
            use->set(ptr);
        }

        // We've replaced all uses of the global var so we should remove it.
        // We leave decorations on the global var above, so if we do not remove it
        // here, the global var will not be processed for elimination in
        // eliminateDeadCode.
        globalVar->removeAndDeallocate();
    }

    IRInst* findOrCreateContextPtrForInst(IRInst* inst)
    {
        // When looking up the context pointer to use for
        // an instruction, we need to find the enclosing
        // function and use whatever context pointer it uses.
        //
        for (IRInst* i = inst; i; i = i->getParent())
        {
            if (auto func = as<IRFunc>(i))
            {
                return findOrCreateContextPtrForFunc(func);
            }
        }

        // If a non-constant global entity is being referenced by
        // something that is *not* nested under an IR function, then
        // we are in trouble.
        //
        SLANG_UNEXPECTED("no outer func at use site for global");
        UNREACHABLE_RETURN(nullptr);
    }

    IRInst* findOrCreateContextPtrForFunc(IRFunc* func)
    {
        // At this point we are being asked to either find or
        // produce a context pointer for use inside `func`.
        //
        // If we already created such a pointer (perhaps because
        // `func` is an entry point), then we are home free.
        //
        if (auto found = m_mapFuncToContextPtr.tryGetValue(func))
        {
            return *found;
        }

        // Otherwise, we are going to need to introduce an
        // explicit parameter to `func` to represent the
        // context.
        //
        IRBuilder builder(m_module);

        // We can safely assume that `func` has a body, because
        // otherwise we wouldn't be getting a request for the
        // context pointer value to use in its body.
        //
        auto firstBlock = func->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        // We create a new parameter at the end of the parameter
        // list for `func`. It points to the `KernelContext` struct, or, in
        // by-value mode, has the hoisted global's value type; `func` then copies
        // it into a local variable whose address stands in for the global.
        //
        auto firstOrdinary = firstBlock->getFirstOrdinaryInst();
        IRParam* contextParam =
            builder.createParam(m_passContextByValue ? m_contextType : m_contextPtrType);
        addKernelContextNameHint(contextParam);
        contextParam->insertBefore(firstOrdinary);
        IRInst* contextPtr = contextParam;
        if (m_passContextByValue)
        {
            builder.setInsertBefore(firstOrdinary);
            contextPtr = builder.emitVar(m_contextType);
            addKernelContextNameHint(contextPtr);
            builder.emitStore(contextPtr, contextParam);
        }

        // Update the type of the function to reflect this new parameter.
        fixUpFuncType(func);

        // The new parameter can be registered as the context value
        // to be used for `func` right away.
        //
        // Note: we register the value *before* modifying locations
        // that call `func` to protect against a possible infinite-recursion
        // situation if `func` is recursive along some path.
        //
        m_mapFuncToContextPtr.add(func, contextPtr);

        // Any code that calls `func` now needs to be updated to pass
        // the context parameter.
        //
        // TODO: There is an issue here if `func` might be called
        // dynamically, through something like a witness table.
        //
        // We collect all the uses first which are in calls.
        // NOTE! That we collect all calls and then process (and don't iterate
        // using the linked list), because when a replacement is made the func usage
        // linked list will no longer hold all of the use sites.
        List<IRCall*> callUses;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            // We will only fix up calls to `func`, and ignore
            // other operations that might refer to it.
            //
            // TODO: We need to allow things like decorations that might
            // refer to `func`, but this logic is also going to
            // ignore things like witness tables that refer to `func`,
            // or operations that pass `func` as a function pointer
            // to a higher-order function.
            //
            auto call = as<IRCall>(use->getUser());
            if (call)
            {
                callUses.add(call);
            }
        }

        // Fix up all of the call uses
        for (auto call : callUses)
        {
            // We are going to construct a new call to `func`
            // that has all of the arguments of the original call...
            //
            UInt originalArgCount = call->getArgCount();
            List<IRInst*> args;
            for (UInt aa = 0; aa < originalArgCount; ++aa)
            {
                args.add(call->getArg(aa));
            }

            // ... plus an additional argument representing
            // the context at the call site, by value or pointer (note that
            // this step leads to a potential for recursion in this pass;
            // the maximum depth of the recursion is bounded by the
            // maximum length of a cycle-free path through the call
            // graph of the program).
            //
            auto callerContextPtr = findOrCreateContextPtrForInst(call);

            // The new call will be emitted right before the old one,
            // then used to replace it.
            //
            builder.setInsertBefore(call);
            args.add(m_passContextByValue ? builder.emitLoad(callerContextPtr) : callerContextPtr);
            auto newCall = builder.emitCallInst(call->getFullType(), call->getCallee(), args);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }

        return contextPtr;
    }

    // Because we have multiple places where instructions representing
    // the kernel context get introduced, we have factored out a subroutine
    // for setting up the name hint to be used by those instructions.
    //
    void addKernelContextNameHint(IRInst* inst)
    {
        IRBuilder builder(m_module);
        builder.addNameHintDecoration(inst, UnownedTerminatedStringSlice("kernelContext"));
    }
};

/// Collect global-scope variables/paramters to form an explicit context that gets threaded through
void introduceExplicitGlobalContext(IRModule* module, CodeGenTarget target)
{
    IntroduceExplicitGlobalContextPass pass(module, target);
    pass.processModule();
}

} // namespace Slang
