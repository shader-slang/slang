// slang-ir-specialize-resources.cpp
#include "slang-ir-specialize-resources.h"

#include "slang-ir-clone.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-specialize-function-call.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// `specializeResourceUsage` prepares resource-valued function inputs and outputs for target
// emission. We first specialize call sites whose resource arguments or results have a target-
// supported form. When callers cannot reconstruct a resource output, we try to inline direct calls
// to the nonrecursive callee, simplify after a successful attempt, and retry specialization. Once
// that step makes no more progress, D3D targets also try to inline a nonrecursive call when a
// resource argument has no HLSL form that the emitter can pass directly. Each successful inlining
// or specialization round is followed by simplification before the process repeats.
// On Khronos targets, each specialization round also replaces resource selections from the same
// heap or array with selections of their indices.

// True for the texture access modes that lower to writable surface accesses
// (RW / write-only / rasterizer-ordered). Read-only (sampled) and sampler-feedback
// textures are deliberately excluded: they do not go through the size-scaling surface
// path. Shared by the GLSL-legality check and the CUDA specialization arm.
static bool isWritableTextureType(IRType* type)
{
    auto texType = as<IRTextureType>(type);
    if (!texType)
        return false;
    switch (texType->getAccess())
    {
    case SLANG_RESOURCE_ACCESS_READ_WRITE:
    case SLANG_RESOURCE_ACCESS_WRITE:
    case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
        return true;
    default:
        return false;
    }
}

/// Return whether `type` can require D3D resource-input inlining.
//
// Resource-global replacement and a resource local lowered from a declaration in the input program
// can both produce the same `IRLoad` argument. D3D legality depends on that IR form rather than the
// local's origin. We reuse the replacement pass's type contract to bound the policy without
// extending it to unrelated parameter-group or resource forms.
static bool isD3DResourceInputInliningCandidateType(IRType* type)
{
    return isResourceValueOrArrayTypeSupportedForPerInvocationReplacement(type);
}

/// Return whether following direct calls from `func` can reach `target`.
//
// Once we have searched every path from a function without reaching `target`, revisiting that
// function through a cycle cannot reveal a new path. Calls through function values are not
// candidates for either inlining step, so the same direct-call graph is the relevant
// graph for this test.
static bool canReachFunctionThroughDirectCalls(
    IRFunc* func,
    IRFunc* target,
    HashSet<IRFunc*>& visitedFuncs)
{
    if (!visitedFuncs.add(func))
        return false;

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            auto call = as<IRCall>(inst);
            auto callee = call ? as<IRFunc>(call->getCallee()) : nullptr;
            if (!callee || !callee->isDefinition())
                continue;
            if (callee == target)
                return true;
            if (canReachFunctionThroughDirectCalls(callee, target, visitedFuncs))
                return true;
        }
    }
    return false;
}

/// Return whether `func` belongs to a cycle in the current direct-call graph.
//
// The resource-output and D3D resource-input inlining steps rescan the module after a successful
// change. If a selected callee is recursive, inlining an external call copies another call into the
// same cycle, and every retry can repeat that expansion. Resource-output specialization can also
// inspect callees recursively before it reaches either inlining step. We therefore exclude
// recursive functions from output specialization and both inliners. We query the current graph at
// each decision because parameter specialization can create new functions and new cycles during
// this pass. GPU targets normally diagnose recursion before this pass; this exclusion also
// guarantees termination when non-essential validation is disabled.
static bool isFunctionRecursive(IRFunc* func)
{
    // We return false for a declaration because it has no outgoing calls in this module.
    if (!func->isDefinition())
        return false;
    HashSet<IRFunc*> visitedFuncs;
    return canReachFunctionThroughDirectCalls(func, func, visitedFuncs);
}

struct ResourceParameterSpecializationCondition : FunctionCallSpecializeCondition
{
    // This pass is intended to specialize functions
    // with resource parameters to ensure that they are
    // legal for a given target.

    TargetRequest* targetRequest = nullptr;
    TargetProgram* targetProgram = nullptr;

    bool doesParamWantSpecialization(IRParam* param, IRInst* arg, IRCall* callInst)
    {
        SLANG_UNUSED(arg);
        SLANG_UNUSED(callInst);

        // Whether or not a parameter needs specialization is really
        // a function of its type:
        //
        IRType* type = param->getDataType();

        // What's more, if a parameter of type `T` would need
        // specialization, then it seems clear that a parameter
        // of type "array of `T`" would also need specialization.
        // We will "unwrap" any outer arrays from the parameter
        // type before moving on, since they won't affect
        // our decision.
        //
        type = unwrapArray(type);
        bool isArray = type != param->getDataType();

        // On all of our (current) targets, a function that
        // takes a `ConstantBuffer<T>` parameter requires
        // specialization. Surprisingly this includes DXIL
        // because dxc apparently does not treat `ConstantBuffer<T>`
        // as a first-class type.
        //
        // TODO: This should not apply to CPU or CUDA, where
        // `ConstantBuffer<T>` is just `T*`. Right now this
        // optimization is not applying to those targets by
        // coincidence (because the shader parameters are not
        // globals, there is no way for the specialization to
        // succeed), but eventually we should turn it off more
        // carefully.
        //
        if (as<IRUniformParameterGroupType>(type))
            return true;

        // For GL/Vulkan targets, we also need to specialize
        // any parameters that use structured or byte-addressed
        // buffers or images with format qualifiers.
        //
        if (isKhronosTarget(targetRequest))
        {
            if (targetProgram->getOptionSet().shouldEmitSPIRVDirectly())
                return isIllegalSPIRVParameterType(type, isArray);
            else
                return isIllegalGLSLParameterType(type);
        }
        else if (isWGPUTarget(targetRequest))
        {
            return isIllegalWGSLParameterType(type);
        }
        else if (isCUDATarget(targetRequest))
        {
            // A writable texture's backing `[format]` decoration is only reachable at the
            // surface-access site through the concrete global, not through an undecorated
            // parameter; specializing the callee to the global restores that reachability
            // (shader-slang/slang#12737), as it already does for Khronos/WGSL.
            if (isWritableTextureType(type))
                return true;
        }

        // For now, we will not treat any other parameters as
        // needing specialization, even if they use resource
        // types like `Texure2D`, because these are allowed
        // as function parameters in both HLSL and GLSL.
        //
        // TODO: We may want to perform more aggressive
        // specialization in general, especially insofar
        // as it could simplify the task of supporting
        // functions with resource-type outputs.

        return false;
    }
};

bool specializeResourceParameters(CodeGenContext* codeGenContext, IRModule* module)
{
    bool result = false;
    ResourceParameterSpecializationCondition condition;
    condition.targetProgram = codeGenContext->getTargetProgram();
    condition.targetRequest = codeGenContext->getTargetReq();
    bool changed = true;
    while (changed)
    {
        changed = specializeFunctionCalls(codeGenContext, module, &condition);
        result |= changed;
    }
    return result;
}

/// Try to inline each direct call to a nonrecursive `func` and return whether any call was changed.
//
// Resource-output specialization records a function when it cannot describe the function's
// resource return or output parameter in a form that callers can reconstruct. Inlining each direct
// call moves that computation into the caller, where SSA simplification can remove intermediate
// storage.
static bool tryInlineDirectCallsOfNonrecursiveFunction(IRFunc* func)
{
    if (isFunctionRecursive(func))
        return false;

    bool changed = false;
    traverseUses(
        func,
        [&](IRUse* use)
        {
            auto user = use->getUser();
            auto call = as<IRCall>(user);
            if (!call)
                return;
            if (call->getCallee() != func)
                return;
            changed |= inlineCall(call);
        });
    return changed;
}

/// Return whether `argument` is an ordinary parameter of the function that contains `call`.
//
// HLSL permits one function to forward a resource parameter to another function. An `IRParam` on a
// later block has a different meaning: it merges values selected by control flow and may therefore
// represent several resource bindings. We accept only parameters on the caller's entry block.
static bool isResourceArgumentForwardedFromCallerParameter(IRCall* call, IRInst* argument)
{
    auto argumentParameter = as<IRParam>(argument);
    if (!argumentParameter)
        return false;

    auto caller = getParentFunc(call);
    if (!caller)
        return false;

    return argumentParameter->getParent() == caller->getFirstBlock();
}

/// Return whether `call` is eligible for an inlining attempt that may eliminate an unsupported D3D
/// resource argument.
//
// Consider this example:
//
//     uniform uint index;
//     static Texture2D<float4> selected;
//
//     float4 loadSelected()
//     {
//         return selected.Load(int3(0));
//     }
//
//     [shader("fragment")]
//     float4 main() : SV_Target
//     {
//         selected = ResourceDescriptorHeap[index];
//         return loadSelected();
//     }
//
// AST-to-IR lowering creates an `IRGlobalVar` for `selected`. `legalizeResourceGlobalVars`
// replaces it with an entry-point `IRVar` and adds a value parameter to the read-only helper.
// The rewritten call can still load its argument from that local before SSA simplification.
// HLSL can pass certain resource expressions directly, including global parameters, indexed
// global resource arrays, and descriptor-heap conversions. An `IRLoad` from the generated local
// does not have one of those recognized forms. When that form remains, we inline `loadSelected`
// and let SSA simplification substitute the descriptor-heap conversion at each use.
static bool isD3DCallEligibleForResourceInputInlining(
    CodeGenContext* codeGenContext,
    ResourceParameterSpecializationCondition& specializationCondition,
    Dictionary<IRFunc*, bool>& isRecursiveByCallee,
    IRCall* call)
{
    auto callee = as<IRFunc>(call->getCallee());
    if (!callee || !callee->isDefinition())
        return false;

    // A target-intrinsic decoration selects emitted code in place of the IR body. Inlining that
    // body would silently discard the selected implementation.
    UnownedStringSlice targetIntrinsicDefinition;
    IRInst* targetIntrinsicInst = nullptr;
    if (findTargetIntrinsicDefinition(
            callee,
            codeGenContext->getTargetReq()->getTargetCaps(),
            targetIntrinsicDefinition,
            targetIntrinsicInst))
    {
        return false;
    }

    // Generic assembly supplies the emitted implementation instead of an ordinary IR body.
    // `inlineCall` cannot inline such a function. We leave the call unchanged so that the late
    // check can diagnose its argument if unsupported mutable storage survives.
    if (hasGenericAssemblyImplementation(callee))
        return false;

    UInt argumentIndex = 0;
    for (auto parameter : callee->getParams())
    {
        SLANG_ASSERT(argumentIndex < call->getArgCount());
        auto argument = call->getArg(argumentIndex++);

        // D3D can pass a resource argument when it is forwarded from the caller's own parameter or
        // when `isParamSuitableForSpecialization` recognizes its IR form. We inline only a by-value
        // resource argument that satisfies neither condition.
        if (!isD3DResourceInputInliningCandidateType(parameter->getDataType()))
            continue;
        if (specializationCondition.isParamSuitableForSpecialization(parameter, argument) ||
            isResourceArgumentForwardedFromCallerParameter(call, argument))
        {
            continue;
        }

        // Inlining a recursive callee would copy another call from the same cycle. Ordinary
        // validation diagnoses that cycle before specialization; when validation is disabled,
        // leaving the call unchanged still guarantees that this inlining step terminates. Several
        // calls can have the same callee. `collectD3DCallsEligibleForResourceInputInlining` passes
        // one cache through the complete module traversal, so we compute each callee's recursion
        // status at most once.
        bool isRecursive;
        if (!isRecursiveByCallee.tryGetValue(callee, isRecursive))
        {
            isRecursive = isFunctionRecursive(callee);
            isRecursiveByCallee.add(callee, isRecursive);
        }
        return !isRecursive;
    }
    return false;
}

/// Collect each direct D3D call eligible for resource-input inlining.
//
// We collect before mutating the IR because inlining can add blocks and clone calls. The outer
// fixed-point loop rescans the resulting module after simplification, so calls introduced by one
// inlining step are considered on the next iteration.
static void collectD3DCallsEligibleForResourceInputInlining(
    CodeGenContext* codeGenContext,
    ResourceParameterSpecializationCondition& specializationCondition,
    Dictionary<IRFunc*, bool>& isRecursiveByCallee,
    IRInst* inst,
    List<IRCall*>& outCalls)
{
    if (auto call = as<IRCall>(inst))
    {
        if (isD3DCallEligibleForResourceInputInlining(
                codeGenContext,
                specializationCondition,
                isRecursiveByCallee,
                call))
        {
            outCalls.add(call);
        }
    }

    for (auto child : inst->getChildren())
    {
        collectD3DCallsEligibleForResourceInputInlining(
            codeGenContext,
            specializationCondition,
            isRecursiveByCallee,
            child,
            outCalls);
    }
}

/// Try to inline eligible D3D calls whose resource arguments cannot be emitted directly as HLSL.
//
// We return whether at least one call was inlined. Calls that `inlineCall` cannot transform remain
// unchanged.
static bool tryInlineD3DCallsWithUnsupportedResourceInputs(
    CodeGenContext* codeGenContext,
    IRModule* module)
{
    if (!isD3DTarget(codeGenContext->getTargetReq()))
        return false;

    ResourceParameterSpecializationCondition specializationCondition;
    specializationCondition.targetProgram = codeGenContext->getTargetProgram();
    specializationCondition.targetRequest = codeGenContext->getTargetReq();

    List<IRCall*> callsToInline;
    {
        // `isFunctionRecursive` searches the current direct-call graph. We pass one cache through
        // the complete module traversal, which does not mutate that graph. The inner scope destroys
        // the cache before the loop below invokes `inlineCall`, which can change the graph.
        Dictionary<IRFunc*, bool> isRecursiveByCallee;
        collectD3DCallsEligibleForResourceInputInlining(
            codeGenContext,
            specializationCondition,
            isRecursiveByCallee,
            module->getModuleInst(),
            callsToInline);
    }

    bool changed = false;
    for (auto call : callsToInline)
    {
        changed |= inlineCall(call);
    }
    return changed;
}

/// A pass to specialize resource-typed function outputs
struct ResourceOutputSpecializationPass
{
    // This pass is kind of a dual to `specializeResourceParameters()`.
    // Whereas that pass identifies call sites that pass suitable argument
    // values and specializes the callee functionfor each such call site,
    // this pass identifies *functions* that *output* suitable values (either
    // via `return` or `out`/`inout` parmeters), and then specializes the
    // *call sites* for those functions based on the values that are output.

    CodeGenContext* codeGenContext;
    TargetRequest* targetRequest;
    IRModule* module;

    /// Functions that requires specialization but are currently unspecializable.
    HashSet<IRFunc*>* unspecializableFuncs;

    /// Functions that required specialization and were specialized.
    HashSet<IRFunc*> specializedFuncs;

    enum class SpecializeFuncResult
    {
        OtherFuncFailed = -2,
        ThisFuncFailed = -1,
        Ok = 1,
    };

    bool failedResult(SpecializeFuncResult val) { return val < SpecializeFuncResult::Ok; }

    bool processModule()
    {
        specializedFuncs.clear();
        bool changed = false;

        // The main logic consists of iterating over all functions
        // (which must appear at the global level) and specializing
        // them if needed.
        //
        for (auto inst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;

            changed |= processFunc(func);
        }
        return changed;
    }

    bool processFunc(IRFunc* oldFunc)
    {
        // Avoid re-computing by checking our 'processFunc' cache.
        if (specializedFuncs.contains(oldFunc))
            return true;
        if (unspecializableFuncs->contains(oldFunc))
            return false;

        // We don't want to waste any effort on functions that don't merit
        // specialization, so the first step is to identify if the function
        // has any outputs that use resource types.
        //
        // If there are no suitable outputs, then we bail out and skip
        // the given function.
        //
        if (!shouldSpecializeFunc(oldFunc))
            return false;

        // Specializing a resource output recursively inspects callees before recording success.
        // A function in a call-graph cycle could therefore re-enter this operation indefinitely.
        // We leave recursive functions unchanged by this output transformation; supported GPU
        // pipelines have already diagnosed recursion, and the pass must still terminate when
        // that validation is disabled.
        if (isFunctionRecursive(oldFunc))
            return false;

        // It is possible that we have a function that we *should* specialize
        // (based on its signature), but we *cannot* yet specialize it.
        //
        // Rather than try to detect that situation as a pre-process, we
        // will instead take the simpler approach of trying to produce
        // a specialized version of `oldFunc`, and bail out if we run
        // into any problems.
        //
        // TODO: It is possible that the allocation we perform here could
        // lead to performance issues if this pass gets iterated. Eventually
        // we should probably merge the resource-based specialization logic
        // into a combined pass that specializes in both directions and
        // also folds in SSA formation to clean up temporaries.

        // We start the specialization process by making a clone of the
        // original function.
        //
        IRBuilder builder(module);
        builder.setInsertBefore(oldFunc);
        IRFunc* newFunc = builder.createFunc();
        newFunc->setFullType(oldFunc->getFullType());

        IRCloneEnv cloneEnv;
        cloneInstDecorationsAndChildren(&cloneEnv, module, oldFunc, newFunc);

        // At first `newFunc` is a direct clone of `oldFunc`, and thus doesn't
        // solve any of our problems. We will traverse `oldFunc` and specialize
        // it as needed, while also collecting information that will allow
        // us to rewrite call sites.
        //
        FuncInfo funcInfo;
        SpecializeFuncResult result = specializeFunc(newFunc, funcInfo);
        if (failedResult(result))
        {
            // Even though we deterined that we *should* specialize
            // this function, we were not able to because of some
            // failure inside the body of the function.
            //
            // For now, we don't treat this as an error condition,
            // because subsequent optimization could make it so
            // that another attempt at this pass succeeds.
            //
            // TODO: We should iterate on this pass and the relevant
            // simplifications, and keep attempting until we hit
            // a steady state, before running this pass one
            // last time with a flag that causes it to emit an
            // error message on this falure path.
            //
            // TODO: Of course, we should *also* have front-end
            // validation that ensures that functions that include
            // targets with limited resource capabilities only use
            // potentially-resource-bearing types in ways that we
            // are sure we can optimize/simplify, so that the error
            // messages can be front-end rather than back-end errors.
            //
            newFunc->removeAndDeallocate();
            // Check if `oldFunc` is the reason for failing,
            // Otherwise don't add to 'unspecializableFuncs'
            //
            // Ensure oldFunc has uses, else, there is nothing to specialize here.
            // If oldFunc has IRKeepAlive, this code should be assumed to have a
            // "dynamic" resource value.
            if (result == SpecializeFuncResult::ThisFuncFailed && oldFunc->hasUses())
                unspecializableFuncs->add(oldFunc);
            return false;
        }

        // Specialization might have changed the signature of `newFunc`,
        // by adding/removing parameters, or by changing the result
        // type it returns.
        //
        // There is a utility function called `fixUpFuncType` that can
        // change the type of an IR function to match its parameter list,
        // but we need to compute the desired result type manually.
        //
        // The result type defaults to the result type of the original
        // function, but should be changed to `void` if specialization
        // was applied to the function result.
        //
        IRType* newResultType = oldFunc->getResultType();
        if (funcInfo.result.flavor != OutputInfo::Flavor::None)
            newResultType = builder.getVoidType();
        fixUpFuncType(newFunc, newResultType);

        // At this point, we generated a `newFunc` that specialized `oldFunc`,
        // and can be used instead of it at any direct call sites.
        //
        // We are going to replace those call sites, which will modify the
        // use-def information for `oldFunc`, so we start by collecting the
        // call sites into an array.
        //
        // Note: We are ignoring any uses that are not direct calls of `oldFunc`;
        // alternative use sites might include references to the function from
        // witness tables, etc. The expectation when using this pass is that
        // any other uses of `oldFunc` will eventually be eliminated, so that
        // only the specialized version remains. If uses of the unspecialized
        // function remain, they could cause problems for downstream code generation.
        //
        // Targets that want to support true dynamic dispatch through witness
        // tables or higher-order functions will need to either disallow resource-type
        // returns from such functions, or support resource-type returns without
        // the aid of this pass.
        //
        List<IRCall*> calls;
        traverseUses(
            oldFunc,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                auto call = as<IRCall>(user);
                if (!call)
                    return;
                if (call->getCallee() != oldFunc)
                    return;
                calls.add(call);
            });

        // Once we have identified the calls to `oldFunc`, we will set about replacing
        // them with calls to `newFunc`.
        //
        // Note: from this point on specialization is not allowed to fail; if the callee
        // function could be specialized then all call sites to it must be specialized.
        // There should be no conditions at call sites that can cause specialization to
        // fail, because specialization does not depend on what is passed *in* to each
        // call, but only on what gets passed *out*.
        //
        for (auto oldCall : calls)
        {
            specializeCallSite(oldCall, newFunc, funcInfo);
        }
        specializedFuncs.add(oldFunc);

        // Since we can no longer fail and we are replacing all `Func` uses, 'KeepAlive'
        // can be removed from the oldFunc so DCE can it clean-up.
        if (auto keepAliveDecoration = oldFunc->findDecoration<IRKeepAliveDecoration>())
            keepAliveDecoration->removeAndDeallocate();
        return true;
    }

    // With the overall flow of the pass described, we can now drill down
    // to the subroutines and data structures that make the whole task possible.
    //
    // We start with the simple problm of deciding whether or not we should
    // (attempt to) specialize a given function.
    //
    bool shouldSpecializeFunc(IRFunc* func)
    {
        // We cannot specialize a function if we do not have
        // access to its definition.
        //
        if (!func->isDefinition())
            return false;
        UnownedStringSlice def;
        IRInst* intrinsicInst;
        if (findTargetIntrinsicDefinition(func, targetRequest->getTargetCaps(), def, intrinsicInst))
            return false;

        // If any of the parameters of the function are `out`
        // or `inout` parameters of a resource type, then we
        // should specialize the function.
        //
        for (auto param : func->getParams())
        {
            auto paramType = param->getDataType();
            auto outType = as<IROutParamTypeBase>(paramType);
            if (!outType)
                continue;
            auto valueType = outType->getValueType();
            if (isResourceType(valueType))
                return true;
        }

        // If the result type of the function is a resource type,
        // then we should specialize the function.
        //
        if (isResourceType(func->getResultType()))
        {
            return true;
        }

        // If the above checks do not trigger, then we don't
        // need/want to specialize the function after all.
        //
        return false;
    }

    // For the above function to work, we need to be able to identify
    // the resource types (and arrays thereof) that require specialization.
    //
    // TODO: It seems like we should be able to share a central definition
    // of resource-ness.
    //
    // Note: we do not worry about parameters/results that are structures
    // with resource-type fields, under the assumption that resource
    // legalization has already been applied, exposing all resource-type
    // parameters as their own top-level parameters.
    //
    // TODO: resource legalization may not apply correctly to function
    // results and `out`/`inout` parameters, in which case we need to
    // fix that pass.
    //
    bool isResourceType(IRType* type)
    {
        type = unwrapArray(type);

        if (as<IRResourceTypeBase>(type))
            return true;

        if (as<IRUniformParameterGroupType>(type))
            return true;

        if (as<IRHLSLStructuredBufferTypeBase>(type))
            return true;

        if (as<IRByteAddressBufferTypeBase>(type))
            return true;

        if (as<IRSamplerStateTypeBase>(type))
            return true;

        // TODO: more cases here?

        return false;
    }

    // Once we've decided that a function is worth specializing,
    // we will both transform the function and collect information
    // about its outputs.
    //
    // The central piece of the data structure we will use is
    // `OutputInfo`, which will track information about one
    // (possible) function output that might need specialization.

    /// Information about a possible output of a function (return value or output parameter)
    struct OutputInfo
    {
        enum class Flavor
        {
            None, ///< Not actually an output, or does not need specialization

            Undefined, ///< Needs specialization, but no suitable replacement value is known

            Replace, ///< A replacement value should be computed based on `representative`
        };

        /// What sort of output value is this?
        Flavor flavor = Flavor::None;

        /// For an output value with the `Replace` flavor, the representative value to clone.
        IRInst* representative = nullptr;

        /// The index of the first new output parameter introduced for this output
        Index firstNewOutputParamIndex = 0;

        /// The number of new output parameters introduced for this output
        Index newOutputParamCount = 0;
    };

    // The function result will be tracked as an `OutputInfo`, and
    // we will define a subtype specific to that case, even though
    // it does not currently need to track any additional data.

    /// A representation of the return-value output of a function
    struct ReturnValueInfo : OutputInfo
    {
    };

    // Parameters can be outputs, so they will also collect information
    // into `OutputInfo`s, but they also need additional information
    // related to the fact that parameters have corresponding arguments
    // at call sites, and how we specialize the parameter affects
    // what we need to do with those arguments.

    /// A representation of a parameter (possibly an output) of a function
    struct ParamInfo : OutputInfo
    {
        /// Represents what to do with an existing argument at a call site.
        enum class OldArgMode
        {
            Keep,   ///< Keep the argument as-is.
            Ignore, ///< Ignore the argument (eliminate it from the call)
            Deref,  ///< Dereference the argument; it used to be `inout` and is now just `in`
        };

        /// What do do with existing arguments at call sites
        OldArgMode oldArgMode = OldArgMode::Keep;
    };

    // It is possible that specializing a function output may require
    // us to add new output parameters to the function, to enable
    // the caller to compute the correct output resource.
    //
    // For example, consider this input:
    //
    //      Texture2D getRandomTexture()
    //      {
    //          int index = /* complicated logic */;
    //          return gTextures[index];
    //      }
    //      ...
    //      Texture2D t = getRandomTexture();
    //
    // The desired output is:
    //
    //      void _getRandomTexture(out int i)
    //      {
    //          int index = /* complicated logic */;
    //          i = index;
    //      }
    //      ...
    //      int i;
    //      getRandomTexture(i);
    //      Texture2D t = gTextures[i];
    //
    // In this case we have made the computation of `t` be
    // valid for targets with limited resource support, but
    // we have kept the complicated computation of `index`
    // in a subroutine, so that we have not bloated the
    // code more than necessary.
    //
    // In order to track new parameters like `i` above,
    // we introduce the `NewOutputParamInfo` type.

    /// Represents a new output parameter introduced during speicalization
    struct NewOutputParamInfo
    {
        /// The type of the new parameter's *value* (not the pointer type for an `out` parameter)
        IRType* type;
    };

    // Finally, we can aggregate the types above to represent the
    // collected information about a function to be specialized.

    /// Information about a function to be specialized
    struct FuncInfo
    {
        ReturnValueInfo result;
        List<ParamInfo> oldParams;
        List<NewOutputParamInfo> newOutputParams;
    };

    // We now turn to the code that fills in the `FuncInfo` structure.

    SpecializeFuncResult specializeFunc(IRFunc* func, FuncInfo& outFuncInfo)
    {
        // To specialize a function, we attempt to specialize
        // all the applicable parameters and the function result.
        //
        // Any failures along the way cause the whole process to fail.

        // Note: We are introducing new parameters at the same time as we
        // iterate over the parameter list, so we cannot just use the
        // `func->getParams()` convenience accessor. Instead, we manually
        // iterate over the parameters in a way that avoids invalidation
        // if we remove the `param` we are working on.
        //
        // Note: it might seem odd that we are modifying `func` but will
        // still bail out on any errors. You might ask: isn't there a chance
        // that we will end up with the function in a partially-modified state?
        //
        // The important thing to remember is that `func` is  *copy* of the
        // original function, so any modifications we make to it do not
        // affect the original, so that if we *do* have to bail out we can
        // leave any call sites intact as calls to the original. The result
        // is that bailing out here may leave the new/copied function in
        // a state where it isn't useful, but it also won't have any uses,
        // and can be eliminated later.
        //
        IRParam* nextParam = nullptr;
        for (IRParam* param = func->getFirstParam(); param; param = nextParam)
        {
            nextParam = param->getNextParam();

            ParamInfo paramInfo;
            auto result = maybeSpecializeParam(param, paramInfo, outFuncInfo);
            if (failedResult(result))
                return result;
            outFuncInfo.oldParams.add(paramInfo);
        }

        auto result = maybeSpecializeResult(func, outFuncInfo.result, outFuncInfo);
        if (failedResult(result))
            return result;

        return SpecializeFuncResult::Ok;
    }

    // The logic for specializing a function result (the return value) is
    // simpler than that for parameters, so we will look at it first.

    SpecializeFuncResult maybeSpecializeResult(
        IRFunc* func,
        ReturnValueInfo& outResultInfo,
        FuncInfo& ioFuncInfo)
    {
        // If the result type of the function isn't a resource type,
        // then we don't need to specialize the result, and we
        // can succeed without doing anything.
        //
        if (!isResourceType(func->getResultType()))
            return SpecializeFuncResult::Ok;

        // Otherwise, we know that we will need to produce specialization
        // information in `outResultInfo` or fail in the attempt.
        //
        // We start with the `prepareOutputValue` subroutine which will
        // handle some common logic shared with the parameter case.
        //
        prepareOutputValue(outResultInfo, ioFuncInfo);

        // Next, we want to identify all the places where the function
        // `return`s a value, since those establish all the possible
        // values for the function result.
        //
        // Specialization will only be possible if all of those results
        // return the "same" value, or values that are in some way
        // similar enough for us to collapse into a single pattern.
        //
        // Identifying the return sites is as simple as looking at
        // the terminator instructions of all blocks in the function.
        //
        for (auto block : func->getBlocks())
        {
            auto returnInst = as<IRReturn>(block->getTerminator());
            if (!returnInst)
                continue;

            auto value = returnInst->getVal();

            IRBuilder builder(module);
            builder.setInsertBefore(returnInst);

            // Given the `value` being returned, we need to determine
            // if it is usable for specializing call sites to this
            // function.
            //
            // If there is a single `return` site, then we can use
            // the value returned there as a representative of the
            // value returned.
            //
            // If there are multiple `return` sites, then any sites
            // after the first will check if they are similar enough
            // in structure to the first one to allow specialization
            // to proceed.
            //
            // If we either fail to identify a specializable result
            // or to match a new `return` value against previous
            // ones, then the specialization process will fail.
            //
            auto result = specializeOutputValue(value, outResultInfo, ioFuncInfo);
            if (failedResult(result))
                return result;

            // We will replace the `return <value>;` operation with
            // a simple `return;`, because the new specialized function
            // will have no return value.
            //
            builder.emitReturn();
            returnInst->removeAndDeallocate();
        }

        // If we have succeeded in gathering information from all
        // the `return` sites, then we can finish up computing
        // `outResultInfo` and return successfully.
        //
        completeOutputValue(outResultInfo, ioFuncInfo);
        return SpecializeFuncResult::Ok;
    }

    void prepareOutputValue(OutputInfo& ioValueInfo, FuncInfo& ioFuncInfo)
    {
        // This function is called when we have identified that a particular
        // value *does* represent an output, but before we have determined
        // what value(s) are used for that output.
        //
        // As such, we set the output into a mode where its value is undefined,
        // since that is the approrpiate default to use in the case where
        // the function doesn't actually write anything to an output.
        //
        ioValueInfo.flavor = OutputInfo::Flavor::Undefined;

        // We also know that the output might require zero or more new output
        // parameters, and we can set the starting index of those parameters
        // based on what (if any) has been generated so far.
        //
        ioValueInfo.firstNewOutputParamIndex = ioFuncInfo.newOutputParams.getCount();
    }

    void completeOutputValue(OutputInfo& ioValueInfo, FuncInfo& ioFuncInfo)
    {
        // This function is called when we are done computing the information
        // required to specialize a particular output value.
        //
        // We can now determine how many new output parameters, if any,
        // were introduced for the sake of this output.
        //
        ioValueInfo.newOutputParamCount =
            ioFuncInfo.newOutputParams.getCount() - ioValueInfo.firstNewOutputParamIndex;
    }

    SpecializeFuncResult specializeOutputValue(
        IRInst* value,
        OutputInfo& ioOutputInfo,
        FuncInfo& ioFuncInfo)
    {
        // This function is called or each `value` that might be written
        // to the output identified by `ioOutputInfo`.

        // If this is the first call to for the given output, then
        // the `representative` value will not have been set.
        //
        IRInst* representative = ioOutputInfo.representative;
        if (!representative)
        {
            // In that case, we will use the given `value` as the
            // representative value of this output.
            //
            representative = value;
            ioOutputInfo.flavor = OutputInfo::Flavor::Replace;
            ioOutputInfo.representative = representative;
        }

        // If this is *not* the first call for the given output,
        // then we need to confirm that `value` and `representative`
        // are suitably matched so that specialization based on `representative`
        // will also suffice for `value`.
        //
        // At the very least, we expect them to be operations with
        // the same opcode.
        //
        if (value->getOp() != representative->getOp())
            return SpecializeFuncResult::ThisFuncFailed;

        // Furthermore, only certain instructions are amenable to
        // specialization, because in general we cannot reproduce
        // an instruction outside of its containing function and
        // have it mean the same thing.
        //
        // We will specifically enumerate the case that we support,
        // and expand them over time.
        //
        // Each supported instruction opcode might introduce new
        // constraints on how `value` and `representative` must match.
        //
        switch (value->getOp())
        {
        default:
            // Any opcode we do not specifically enable should cause
            // specialization to fail.
            //
            return SpecializeFuncResult::ThisFuncFailed;

        case kIROp_GlobalParam:
            // A direct reference to a global shader parameter is
            // the easiest case to handle.
            //
            // We do need to require that all values used for the
            // same output refer to the *same* global parameter.
            //
            if (value != representative)
                return SpecializeFuncResult::ThisFuncFailed;
            return SpecializeFuncResult::Ok;

            // TODO: There are a number of additional cases that we should
            // enable here.
            //
            // The most obvious new cases to support are:
            //
            // * Function parameters: if the output value is one of the
            //   parameter of the function, then callers can just use the
            //   same value they passed for the corresponding argument.
            //
            // * Array indexing: if the array itself is suitable to specialize,
            //   then it should be possible to return the array index via
            //   a new `out` parameter, and have the caller do the indexing.
        }

        // Note: the `FuncInfo` is currently being passed in in aid of the
        // array-indexing case, but because that case is not implemented
        // the parameter is not being used.
        //
        SLANG_UNUSED(ioFuncInfo);

        // TODO: One of the hardest cases here would be `inout` parameters
        // of texture type, where the result value depends on the input value(s):
        //
        //      void swap(inout Texture2D a, inout Texture2D b)
        //      {
        //          Texture2D tmp = a;
        //          a = b;
        //          b = tmp;
        //      }
        //
        // In such a case, the value written to `a` is a `load` from parameter `b`,
        // but it would be difficult to *prove* that such a load represents the
        // value of the parameter on input to the function, rather than on output.
        //
        // It might be best if resource type legalization replaced `inout`
        // parameters of resource type with distinct `in` and `out` parameters,
        // to make the relationships more clear.
    }

    // As discussed earlier, the case for `out`/`inout` function parameters
    // is more involved than that for the function `return` value, so we
    // put it off until we'd discussed the shared subroutines.

    SpecializeFuncResult maybeSpecializeParam(
        IRParam* param,
        ParamInfo& outParamInfo,
        FuncInfo& ioFuncInfo)
    {
        // We only want to specialize in the case where the parameter
        // is an `out` or `inout` (both inherit from `IROutParamTypeBase`),
        // and the pointed-to type is a resource.
        //
        auto paramType = param->getDataType();
        auto outType = as<IROutParamTypeBase>(paramType);
        if (!outType)
            return SpecializeFuncResult::Ok;
        auto valueType = outType->getValueType();
        if (!isResourceType(valueType))
            return SpecializeFuncResult::Ok;

        prepareOutputValue(outParamInfo, ioFuncInfo);

        // We are going to remove the parameter and add zero or more
        // replacements, and we want any replacements to end up
        // at the same point in the function signature.
        //
        IRBuilder paramsBuilder(module);
        paramsBuilder.setInsertBefore(param);

        // We also need to introduce new instructions into the function
        // body, as part of the entry block.
        //
        IRBlock* block = as<IRBlock>(param->getParent());
        IRBuilder bodyBuilder(module);
        bodyBuilder.setInsertBefore(block->getFirstOrdinaryInst());

        // No matter what, we create a local variable that will be
        // used to replace the parameter.
        //
        IRVar* newVar = bodyBuilder.emitVar(valueType);

        if (as<IRBorrowInOutParamType>(outType))
        {
            // If the parameter is an `inout` rather than just
            // an `out`, then we still need a parameter to
            // be passed in, but it can be an `in` parameter
            // instead, which means a `T` instead of an
            // `InOut<T>`.
            //
            IRInst* newParam = paramsBuilder.createParam(valueType);
            newParam->insertBefore(param);
            param->transferDecorationsTo(newParam);

            // The start of the function body should assign
            // from the `in` parameter to the local variable.
            //
            bodyBuilder.emitStore(newVar, newParam);

            // We also need call sites to pass in an argument
            // for the new `in` parameter, which will have to
            // be dereferenced by one level from the original
            // argument they were passing.
            //
            outParamInfo.oldArgMode = ParamInfo::OldArgMode::Deref;
        }
        else
        {
            // The case for a pure `out` parameter is easier:
            // we don't need to initialize the local variable,
            // and we don't need callers to pass in anything.
            //
            outParamInfo.oldArgMode = ParamInfo::OldArgMode::Ignore;
        }

        // Before we change something (and likely break this
        // function if something fails after a change) we want
        // to identify all the places in the function
        // that `store` to the given output parameter.
        //
        // Note: this logic is subtly depending on the structure
        // of how the front-end generates code for `out` and `inout`
        // parameters:
        //
        // * The only `load` of an `inout` parameter is emitted at
        //   the very start of a function body, to copy it over to
        //   a temporary variable.
        //
        // * The only `store`s of an `out` or `inout` parameter are
        //   right before `return` instructions, to establish the
        //   final value of that parameter, and every `out`/`inout`
        //   parameter is stored along every control-flow path
        //   that reaches a `return`.
        //
        // Those invariants could easily be eliminated in a few different
        // ways. Notably, if we added some more clever memory optimizations,
        // then a pass could notice that we have:
        //
        //      let val = load(inoutParam);
        //      ...
        //      store(inoutParam, val);
        //
        // and optimize away the `store` (at least).
        //
        // For now we can get away with this because we don't do very many
        // interesting memory/pointer optimizations in Slang, but it is
        // still worrying to have this kind of assumption baked in.
        //
        // TODO: We should decide on an encoding for the behavior of
        // `out`/`inout` parameters that doesn't have as many "gotcha" cases.
        //
        // We will also now recursively specialize all `IRCall` inside a 'parent function'
        // when trying to specialize a 'parent function'. This is to ensure we do not remove
        // a parameter SSA needs for SSA'ing a localVar into a globalVar (and DCE requires
        // to not DCE an important 'IRCall').
        //
        SpecializeFuncResult recursiveSpecializationResult = SpecializeFuncResult::Ok;
        List<IRStore*> stores;

        // We'll first specialize any relevant calls that may affect the value stored into the
        // param. This may create more stores into the param.
        //
        traverseUses(
            param,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                switch (user->getOp())
                {
                case kIROp_Call:
                    {
                        // This call may require an inline if it fails to specialize
                        IRFunc* func = as<IRFunc>(as<IRCall>(user)->getCallee());
                        if (!func)
                            return;

                        if (!processFunc(func))
                        {
                            recursiveSpecializationResult = SpecializeFuncResult::OtherFuncFailed;
                        }
                        return;
                    }
                default:
                    return;
                };
            });

        // If any call specialization fails, we may need to revisit this function at a later
        // iteration.
        if (failedResult(recursiveSpecializationResult))
            return recursiveSpecializationResult;

        // Then, traverse all stores into this param.
        traverseUses(
            param,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                switch (user->getOp())
                {
                case kIROp_Store:
                    {
                        auto store = as<IRStore>(user);
                        if (store->ptr.get() != param)
                            return;
                        stores.add(store);
                        return;
                    }
                default:
                    return;
                };
            });

        // Having identified the places where a value is stored to
        // the output parameter, we iterate over those values to
        // ensure that they are all specializable and consistent
        // with one another.
        //
        for (auto store : stores)
        {
            auto value = store->val.get();
            auto result = specializeOutputValue(value, outParamInfo, ioFuncInfo);
            if (failedResult(result))
                return result;

            // Given our assumptions about how `store`s to output
            // parameters are used, we can eliminate all these `store`s
            // since the values they write won't ever be used.
            //
            store->removeAndDeallocate();
        }

        // It is possible that there will still be used of the parameter
        // even after we eliminate all the `store`s from it (e.g., the initial
        // `load` that pulls the value from an `inout` parameter), so we
        // replace any remaining uses of the parameter with the local
        // variable we introduced, before removing the parameter.
        //
        param->replaceUsesWith(newVar);
        param->removeAndDeallocate();

        completeOutputValue(outParamInfo, ioFuncInfo);
        return SpecializeFuncResult::Ok;
    }

    void specializeCallSite(IRCall* oldCall, IRFunc* newFunc, FuncInfo const& funcInfo)
    {
        // Given an existing call, we will insert a new call right before
        // it and then remove the old one.
        //
        IRBuilder builder(module);
        builder.setInsertBefore(oldCall);

        // The new callee may have additional `out` parameters that
        // represent things like array indices required by the
        // new lookup operations. The new call site will need
        // to introduce temporaries to capture the values of
        // these outputs.
        //
        List<IRVar*> newOutputVars;
        for (auto const& newOutputParamInfo : funcInfo.newOutputParams)
        {
            auto newOutputVar = builder.emitVar(newOutputParamInfo.type);
            newOutputVars.add(newOutputVar);
        }

        // Next, we need to build up the argument list for
        // the call, by looking at the information that
        // was recorded for each parameter of the original.
        //
        List<IRInst*> newArgs;
        Index oldParamCounter = 0;
        for (auto const& oldParamInfo : funcInfo.oldParams)
        {
            // We can grab the argument from the old call
            // that was being used for this parameter, but
            // we need to check whether or not the new call
            // will use it.
            //
            auto oldParamIndex = oldParamCounter++;
            auto oldArg = oldCall->getArg(oldParamIndex);

            // Depending on how the callee specialized this parameter,
            // we will pass the argument, or data derived from it,
            // or nothing.
            //
            switch (oldParamInfo.oldArgMode)
            {
            default:
                SLANG_UNEXPECTED("unhandled case");
                break;

            case ParamInfo::OldArgMode::Keep:
                // If the parameter was not specialized away, then
                // the argument should be passed as-is.
                //
                newArgs.add(oldArg);
                break;

            case ParamInfo::OldArgMode::Ignore:
                // If the parameter was specialized out of existence,
                // then we don't pass the argument in at all.
                //
                break;

            case ParamInfo::OldArgMode::Deref:
                // If an `inout` argument has been specialized into an
                // `in` argument, then we need to dereference the pointer
                // that was being passed in before, and pass in the value
                // it points to instead.
                //
                // Note: the expectation is that once the call site has
                // been specialized, subsequent optimization will eliminate
                // this `load`, and replace it with whatever value was
                // being stored for the `inout` argument before the call.
                //
                newArgs.add(builder.emitLoad(oldArg));
                break;
            }

            // A resource parameter that got specialized might also introduce new
            // `out` parameters that help the caller compute the right result
            // value (e.g., array indices). Those parameters will come right
            // in the parameter list right after the location of the original
            // parameter.
            //
            for (Index i = 0; i < oldParamInfo.newOutputParamCount; ++i)
            {
                newArgs.add(newOutputVars[oldParamInfo.firstNewOutputParamIndex + i]);
            }
        }

        // The function return value can also require new `out` parameters as
        // part of specialization; any parameters it introduces will go
        // over all the others.
        //
        for (Index i = 0; i < funcInfo.result.newOutputParamCount; ++i)
        {
            newArgs.add(newOutputVars[funcInfo.result.firstNewOutputParamIndex + i]);
        }

        // Once we've built up the argument list for the new call we can emit
        // it, and also transfer any helpful decorations from the old call
        // over to the new one.
        //
        auto newResultType = newFunc->getResultType();
        auto newCall = builder.emitCallInst(newResultType, newFunc, newArgs);
        oldCall->transferDecorationsTo(newCall);

        // Just calling the specialized function is not enough, of course,
        // because the whole point of this pass was to move the logic
        // that computes a result resource from the callee up to the caller.
        //
        // After the call is completed, any additional `out` arguments will
        // have had their values filled in (e.g., the callee will have
        // computed the array index to be used, etc.).
        //
        // We can now iterate over the parameters again, and identify
        // the output parameters that have been specialized.
        //
        oldParamCounter = 0;
        for (auto const& oldParamInfo : funcInfo.oldParams)
        {
            auto oldParamIndex = oldParamCounter++;
            auto oldArg = oldCall->getArg(oldParamIndex);

            // We skip over parameters that were not specialized.
            //
            if (oldParamInfo.flavor == OutputInfo::Flavor::None)
                continue;

            if (oldParamInfo.flavor == OutputInfo::Flavor::Undefined)
                continue;

            // For any paraemter that was specialized, we will use
            // the computed information on the parameter to materialize
            // a value for the output in the context of the caller.
            //
            auto value = materialize(builder, oldParamInfo);

            // For an `out` or `inout` parameter, the `oldArg` represents
            // a pointer where the output value should be stored, so
            // we can emulate the behavior of the original function
            // by storing the value as expected.
            //
            builder.emitStore(oldArg, value);
        }

        // If the function result is an output that needs to be
        // specialized, then we need to handle it much like the
        // parameter case above.
        //
        if (funcInfo.result.flavor != OutputInfo::Flavor::None)
        {
            // We materialize the expected function result into
            // an IR value in the context of the caller, and then
            // use that value to replace any uses of the return
            // value of the original call.
            //
            auto value = materialize(builder, funcInfo.result);
            oldCall->replaceUsesWith(value);
        }
        else
        {
            // If the call was specialized, but the return value
            // was not something that needed specialization, then
            // we still need to replace any uses of the original
            // call to use the value of the new call.
            //
            oldCall->replaceUsesWith(newCall);
        }

        // After we've fully wired up the new call, we eliminate
        // the old call site, which will have no more uses.
        //
        // Note: At this point, the body of the caller function
        // is likely to have opportunities for further optimization.
        // Simple dataflow optimizations should now be able to
        // resolve the identities of resources that had previously
        // only been visible as the value of local variables or
        // the results of `call` instructions.
        //
        oldCall->removeAndDeallocate();
    }

    // In order to specialize call sites to functions that output
    // resources, we need a way to materialize the value for an
    // output in the context of the caller, based on the information
    // that was gathered in the callee.

    IRInst* materialize(IRBuilder& builder, OutputInfo const& info)
    {
        // For now, we are only handling a small fraction of the
        // possible cases.

        SLANG_UNUSED(builder);

        // The basic idea is to look at the `representative` instruction
        // that stands in for the output value (which is an instruction
        // from the body of the callee), and to produce an equivalent
        // value in the context of the caller.
        //
        auto representative = info.representative;
        switch (representative->getOp())
        {
        default:
            // Because we only allow certain instructions when specializing
            // the callee, any instruction outside of the allowed ones
            // would represent an internal error.
            //
            SLANG_UNEXPECTED("unhandled case");
            UNREACHABLE_RETURN(nullptr);

        case kIROp_GlobalParam:
            // If the value in the callee was a reference to a global parameter,
            // then we can simply refer to the same parameter here in the caller.
            //
            return representative;

            // TODO: As other cases are added to `specializeOutputValue()`, we will
            // need to add corresponding cases here.
        }
    }
};

/// Return whether the target backend relies on this pass to remove resource-valued returns and
/// `out` or `inout` parameters before emission.
static bool doesTargetRequireResourceOutputSpecialization(TargetRequest* targetRequest)
{
    if (isD3DTarget(targetRequest))
        return true;
    if (isKhronosTarget(targetRequest))
        return true;
    if (isWGPUTarget(targetRequest))
        return true;
    if (isMetalTarget(targetRequest))
        return true;
    return false;
}

/// Try to replace resource-valued returns and `out` or `inout` parameters, and return whether any
/// function was changed.
//
// We add each function whose resource return or output parameter cannot be reconstructed at its
// call sites to `unspecializableFuncs` without changing that function. `specializeResourceUsage`
// can then try to inline each direct call to it, simplify the module, and retry.
static bool specializeResourceOutputs(
    CodeGenContext* codeGenContext,
    IRModule* module,
    HashSet<IRFunc*>& unspecializableFuncs)
{
    auto targetRequest = codeGenContext->getTargetReq();
    if (!doesTargetRequireResourceOutputSpecialization(targetRequest))
        return false;

    ResourceOutputSpecializationPass pass;
    pass.codeGenContext = codeGenContext;
    pass.targetRequest = targetRequest;
    pass.module = module;
    pass.unspecializableFuncs = &unspecializableFuncs;
    return pass.processModule();
}

// Each resource select gets a corresponding integer select that chooses its resource index.
// Leaf accesses and block parameters must already be mapped. The worklist is consumed.
static void emitResourceIndexSelects(
    List<IRSelect*>& workList,
    IRType* indexType,
    Dictionary<IRInst*, IRInst*>& mapResourceToIndex)
{
    // The integer selects are built in dependency order: in select(c, a, b), where b is another
    // select that also uses a, both users need a's integer replacement first. Discovery order
    // alone does not establish that order, so the worklist finishes missing dependencies first.
    // Cycles pass through block parameters, whose replacements already exist.
    while (workList.getCount())
    {
        auto select = workList.getLast();
        if (mapResourceToIndex.containsKey(select))
        {
            workList.removeLast();
            continue;
        }
        auto trueValue = select->getTrueResult();
        auto falseValue = select->getFalseResult();
        auto trueIndex = mapResourceToIndex.tryGetValue(trueValue);
        auto falseIndex = mapResourceToIndex.tryGetValue(falseValue);
        IRInst* missingValue = nullptr;
        if (!trueIndex)
            missingValue = trueValue;
        else if (!falseIndex)
            missingValue = falseValue;
        if (missingValue)
        {
            // Leaves and parameters already have indices, so only another select can
            // be missing.
            workList.add(cast<IRSelect>(missingValue));
            continue;
        }
        IRBuilder builder(select);
        builder.setInsertBefore(select);
        IRInst* operands[] = {select->getCondition(), *trueIndex, *falseIndex};
        auto index = builder.emitIntrinsicInst(indexType, kIROp_Select, 3, operands);
        index->sourceLoc = select->sourceLoc;
        mapResourceToIndex.add(select, index);
        workList.removeLast();
    }
}

// Incoming branch arguments for parameters in mapResourceToIndex are replaced with their
// mapped integer indices; other arguments remain unchanged. Every replaced argument must
// also have an entry in mapResourceToIndex. originalParams holds the parameter lists from
// before integer parameters were inserted, so params[i] still identifies argument i.
static void rewriteResourceSelectionBranchArgs(
    const Dictionary<IRBlock*, List<IRParam*>>& originalParams,
    const Dictionary<IRInst*, IRInst*>& mapResourceToIndex)
{
    for (const auto& [block, params] : originalParams)
    {
        // Predecessor iteration follows uses of the target block. Updating resource
        // arguments leaves those uses unchanged, so the iteration remains valid.
        // Loop break/continue operands and branch decorations are also unchanged.
        for (auto predecessor : block->getPredecessors())
        {
            auto branch = cast<IRUnconditionalBranch>(predecessor->getTerminator());
            IRBuilder builder(branch);
            // originalParams records the parameter positions before integer parameters were
            // inserted, so params[i] still identifies argument i on every incoming branch.
            //
            // Consider a loop header with parameters (a, b, k), where a and b select samplers from
            // different arrays and we are specializing a's graph. The back edge
            //
            //     unconditionalBranch(%header, %nextA, %nextB, %nextK)
            //
            // becomes
            //
            //     unconditionalBranch(%header, %nextAIndex, %nextB, %nextK)
            //
            // The slot for b keeps its resource because b belongs to another graph, and k is not
            // part of any selection.
            for (Index i = 0; i < params.getCount(); i++)
            {
                // A parameter outside this graph keeps its argument even when that argument
                // is part of the graph. In `a = shared; b = shared;`, specializing a's graph
                // must leave b's slot holding a resource. The caller's replaceUsesWith points
                // that slot at the reconstructed resource, and a later call specializes b.
                if (!mapResourceToIndex.containsKey(params[i]))
                    continue;
                auto index = mapResourceToIndex.getValue(branch->getArg((UInt)i));
                builder.replaceOperand(branch->getArgs() + i, index);
            }
        }
    }
}

// Resource selections become index selections when all incoming values access the same
// heap or array. For example, `c ? samplers[i] : samplers[j]` becomes `samplers[c ? i : j]`.
//
// The function returns true if the graph rooted at candidate was rewritten. Every select
// and block parameter consumed by the rewrite is removed from remainingCandidates.
static bool specializeResourceSelection(
    TargetRequest* targetReq,
    IRInst* candidate,
    OrderedHashSet<IRInst*>& remainingCandidates)
{
    // Array subscripts can have different integer types, so the selected indices need a common
    // type. For `select(c, samplers[i], samplers[j])`, where `i` is int and `j` is uint,
    // the transformation is:
    //
    //     // Before:
    //     let %a : SamplerState = getElement(%samplers, %i : Int)
    //     let %b : SamplerState = getElement(%samplers, %j : UInt)
    //     let %s : SamplerState = select(%c, %a, %b)
    //
    //     // After (unused resource accesses omitted):
    //     let %iAsUInt : UInt = intCast(%i)
    //     let %s : UInt = select(%c, %iAsUInt, %j)
    //     let %index : UInt = nonUniformResourceIndex(%s)
    //     let %sampler : SamplerState = getElement(%samplers, %index)
    //
    // The original index calculations keep their types; we convert their results before the
    // selection. If an incoming index is 64-bit, the common type must also be 64-bit to avoid
    // truncating it. Selecting a descriptor does not change address calculations inside that
    // resource, which are handled separately.
    //
    // A loop can make this more complicated than matching the two operands of a select.
    // The following loop illustrates why:
    //
    //     SamplerState samplers[2];
    //     Texture2D<float4> texture;
    //
    //     float4 sampleTexture(uint count, float2 uv)
    //     {
    //         SamplerState s = samplers[0];
    //         for (uint k = 0; k < count; k++)
    //         {
    //             if (k % 2 == 0)
    //                 s = samplers[1];
    //         }
    //         return texture.SampleLevel(s, uv, 0);
    //     }
    //
    // After SSA construction, the loop header has a block parameter for `s`. Its incoming
    // values are samplers[0] and the result of the if statement. The merge after the if has
    // another block parameter, which receives either samplers[1] or the loop header's `s`.
    // Each parameter therefore depends on the other. Neither can be handled by a rule that
    // requires every incoming value to already be an array access.
    //
    // We follow both block parameters and selects until we reach the resource accesses. If
    // they all use the same source and resource type, we can change the entire selection
    // to use indices of a common integer type. The relevant IR changes look like this (other
    // instructions omitted):
    //
    //     // Before:
    //     block %header(param %s : SamplerState, ...):
    //         ...
    //     block %assign:
    //         let %newS : SamplerState = getElement(%samplers, 1 : Int)
    //         unconditionalBranch(%merge, %newS)
    //     block %keep:
    //         unconditionalBranch(%merge, %s)
    //     block %merge(param %nextS : SamplerState):
    //         ...
    //     unconditionalBranch(%header, %nextS, ...)
    //
    //     // After (unused resource accesses omitted):
    //     block %header(param %s : Int, ...):
    //         let %index : Int = nonUniformResourceIndex(%s)
    //         let %sampler : SamplerState = getElement(%samplers, %index)
    //         ...
    //     block %assign:
    //         unconditionalBranch(%merge, 1 : Int)
    //     block %keep:
    //         unconditionalBranch(%merge, %s)
    //     block %merge(param %nextS : Int):
    //         ...
    //     unconditionalBranch(%header, %nextS, ...)
    //
    // The sample operation now uses %sampler. The branches, including the loop back edge,
    // still pass arguments in the same positions, but carry indices instead of resources.
    // Each select and block parameter is replaced by a new integer instruction. We create
    // all integer parameters first, so the loop dependencies have values to refer to, then
    // create integer selects in dependency order. Reconstructed resource accesses supply
    // the original resource uses, while the branches pass the corresponding indices.
    // We check all incoming values before changing any instruction, so a rejected case leaves
    // the IR intact.

    // An array of resources has no single index for a select to choose.
    auto resourceType = candidate->getDataType();
    if (!isResourceType(resourceType) || as<IRArrayTypeBase>(resourceType))
        return false;

    // The worklist contains resource values that contribute to the original selection.
    // Selects contribute their two values, and parameters contribute incoming branch
    // arguments. Resource accesses end the traversal. Any unsupported producer rejects
    // the entire graph before we change IR.
    List<IRInst*> workList;
    HashSet<IRInst*> visited;
    List<IRInst*> graphNodes;
    Dictionary<IRBlock*, List<IRParam*>> originalParams;
    // Initially this maps only resource accesses to their index operands. After analysis,
    // we convert those indices and add the new integer parameters and selects.
    Dictionary<IRInst*, IRInst*> mapResourceToIndex;
    IROp accessOp = kIROp_Invalid;
    IRInst* accessSource = nullptr;
    IRType* commonIndexType = nullptr;
    workList.add(candidate);
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto inst = workList[i];
        // We have already checked a repeated value. In particular, a loop back edge
        // must not cause us to visit the same parameter indefinitely.
        if (!visited.add(inst))
            continue;
        if (inst->getDataType() != resourceType)
            return false;

        if (auto param = as<IRParam>(inst))
        {
            auto block = cast<IRBlock>(param->getParent());
            auto func = cast<IRFunc>(block->getParent());
            // A function parameter receives its resource from a caller rather than from
            // branch arguments, so this graph does not tell us which heap or array
            // supplies it.
            if (block == func->getFirstBlock())
                return false;
            // A non-entry block with no predecessors is unreachable. There is no incoming
            // resource from which to determine an index.
            if (block->getPredecessors().isEmpty())
                return false;
            // For other block parameters, the argument at the parameter's position on
            // each incoming branch supplies one incoming resource value.
            auto paramIndex = getParamIndexInBlock(param);
            for (auto predecessor : block->getPredecessors())
            {
                auto branch = cast<IRUnconditionalBranch>(predecessor->getTerminator());
                workList.add(branch->getArg((UInt)paramIndex));
            }
            if (!originalParams.containsKey(block))
            {
                auto& params = originalParams[block];
                for (auto originalParam : block->getParams())
                    params.add(originalParam);
            }
        }
        else if (auto select = as<IRSelect>(inst))
        {
            workList.add(select->getTrueResult());
            workList.add(select->getFalseResult());
        }
        else
        {
            // The supported leaves are built-in heap loads and direct global-array
            // accesses. Each exposes the source and index needed to recreate a resource.
            IRInst* source = nullptr;
            IRInst* index = nullptr;
            switch (inst->getOp())
            {
            case kIROp_SPIRVLoadDescriptorFromHeap:
                {
                    auto load = cast<IRSPIRVLoadDescriptorFromHeap>(inst);
                    source = load->getHeap();
                    index = load->getIndex();
                    // Built-in heaps are available throughout the function, so every
                    // reconstructed load can name the same heap.
                    if (!as<IRSPIRVResourceHeap>(source) && !as<IRSPIRVSamplerHeap>(source))
                        return false;
                    break;
                }
            case kIROp_GetElement:
                {
                    auto getElement = cast<IRGetElement>(inst);
                    source = getElement->getBase();
                    index = getElement->getIndex();
                    // Only texture and sampler arrays are rewritten. Buffer arrays such as
                    // StructuredBuffer<T>[] already support selection through variable
                    // pointers, and rewriting them would add nonuniform descriptor-indexing
                    // requirements. Heap loads above are not restricted this way: heap
                    // buffers still need the index rewrite.
                    if (!as<IRResourceTypeBase>(resourceType) &&
                        !as<IRSamplerStateTypeBase>(resourceType))
                        return false;
                    // A global parameter is visible everywhere, so the reconstructed access
                    // can name the same array at every selection. Resource-type legalization
                    // runs before this pass and splits resource fields of global shader
                    // parameters into their own global parameters, so arrays nested in
                    // structs also arrive here as global parameters. Loaded or computed
                    // array values are not rematerialized by this pass.
                    if (!as<IRGlobalParam>(source))
                        return false;
                    // GetElement also supports other aggregate types. This rewrite handles
                    // descriptor arrays, whose elements can be selected by a nonuniform index.
                    if (!as<IRArrayTypeBase>(source->getDataType()))
                        return false;
                    break;
                }
            default:
                return false;
            }
            if (accessOp != kIROp_Invalid)
            {
                // One access operation must reconstruct every selected resource, so we
                // cannot combine heap loads and array indexing in the same graph.
                if (inst->getOp() != accessOp)
                    return false;
                // a[i] and a[j] can share a selected index, but a[i] and b[j] cannot.
                if (source != accessSource)
                    return false;
            }
            accessOp = inst->getOp();
            accessSource = source;
            // Valid array indices are nonnegative. The widest incoming type can hold all
            // of them if we prefer unsigned when widths match. For example, Int and UInt
            // use UInt, while UInt and Int64 use Int64. This preserves existing 64-bit
            // indices without introducing 64-bit values when all incoming indices are narrower.
            auto incomingIndexType = index->getDataType();
            if (!commonIndexType)
                commonIndexType = incomingIndexType;
            else
            {
                auto commonInfo = getIntTypeInfo(targetReq, commonIndexType);
                auto incomingInfo = getIntTypeInfo(targetReq, incomingIndexType);
                if (incomingInfo.width > commonInfo.width ||
                    (incomingInfo.width == commonInfo.width && !incomingInfo.isSigned))
                    commonIndexType = incomingIndexType;
            }
            mapResourceToIndex.add(inst, index);
            continue;
        }
        graphNodes.add(inst);
    }

    // A cycle without any descriptor access does not establish a source to rematerialize.
    // For example, the parameters of an unreachable loop may only feed each other.
    if (accessOp == kIROp_Invalid)
        return false;

    // Casts are inserted only after the whole graph has been accepted, so rejection leaves
    // the IR unchanged. Each conversion precedes its original access, where the index is
    // available, and dominates every selection edge that used that resource, including
    // loop back edges. The loop updates the map in place, so each access then maps to its
    // converted index.
    for (auto& [resource, index] : mapResourceToIndex)
    {
        IRBuilder builder(resource);
        builder.setInsertBefore(resource);
        index = builder.emitCast(commonIndexType, index);
    }

    // New parameters give cyclic dependencies their integer values before we build any
    // selects. We insert each integer parameter immediately before its resource parameter,
    // so the integer parameters take the original positions once the resource parameters
    // are removed. Until then, originalParams records those positions for the branch rewrite.
    List<IRSelect*> pendingSelects;
    for (auto inst : graphNodes)
    {
        if (as<IRParam>(inst))
        {
            IRBuilder builder(inst);
            auto index = builder.createParam(commonIndexType);
            index->insertBefore(inst);
            index->sourceLoc = inst->sourceLoc;
            // The reconstruction step below moves the name hint to this integer parameter.
            // Other decorations follow the reconstructed resource, which retains the original type.
            mapResourceToIndex.add(inst, index);
        }
        else
            pendingSelects.add(cast<IRSelect>(inst));
    }

    emitResourceIndexSelects(pendingSelects, commonIndexType, mapResourceToIndex);

    // We rewrite branch arguments before replaceUsesWith below, because afterwards they
    // no longer name the original resources.
    rewriteResourceSelectionBranchArgs(originalParams, mapResourceToIndex);

    // The new integer graph never uses the old resource selections, so replacing their
    // uses cannot introduce a self-reference. When inst is a block parameter,
    // setInsertBeforeOrdinaryInst places its reconstructed access after all of the block's
    // parameters.
    for (auto inst : graphNodes)
    {
        IRBuilder builder(inst);
        setInsertBeforeOrdinaryInst(&builder, inst);
        IRInst* index = mapResourceToIndex.getValue(inst);
        // The name follows the selected state, which is now an integer. Moving the hint
        // avoids giving the reconstructed access the same source name. Other decorations
        // stay on the reconstructed value, which still has the resource type.
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
        {
            nameHint->removeFromParent();
            nameHint->insertAtStart(index);
        }
        IRInst* resource;
        if (accessOp == kIROp_GetElement)
        {
            // The index used for array indexing can be nonuniform even when all incoming
            // indices are constant: the condition or control flow chooses between them.
            index = builder.emitNonUniformResourceIndexInst(index);
            resource = builder.emitElementExtract(resourceType, accessSource, index);
        }
        else
        {
            // SPIRVLoadDescriptorFromHeap needs no marker: DescriptorHeapEXT permits
            // nonuniform resource access by default.
            resource = builder.emitLoadDescriptorFromHeap(resourceType, accessSource, index);
        }
        resource->sourceLoc = inst->sourceLoc;
        inst->transferDecorationsTo(resource);
        inst->replaceUsesWith(resource);
    }

    for (auto inst : graphNodes)
    {
        // A single graph can consume several candidates saved by the outer traversal.
        // Removing them before deletion prevents that traversal from using stale pointers.
        remainingCandidates.remove(inst);
        inst->removeAndDeallocate();
    }
    // Original resource accesses, and recreated resources used only by other selections,
    // may now be unused. The simplifyIR call in specializeResourceUsage removes them.
    return true;
}

// This pass finds resource-valued selects and block parameters in function bodies and
// replaces supported graphs with index selections. Function parameters are handled by
// specializeResourceParameters.
static bool specializeResourceSelections(CodeGenContext* codeGenContext, IRModule* module)
{
    // Both SPIR-V and GLSL need this rewrite for sampler and texture selections.
    // Out-of-SSA conversion would introduce function-local variables of these types,
    // which neither target can represent. Selecting an integer index lets us load
    // the resource from its heap or array instead.
    // The access instructions and nonuniform-index handling used here cover Khronos targets.
    // WGSL also restricts resource locals, but extending this pass to it requires checking
    // its resource-array and nonuniform-indexing rules; that is not handled here.
    if (!isKhronosTarget(codeGenContext->getTargetReq()))
        return false;

    bool changed = false;
    for (auto global : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(global);
        if (!func)
            continue;
        // We collect possible resource selections represented by selects and non-entry block
        // parameters. A rewrite may consume several candidates, so collection happens first.
        OrderedHashSet<IRInst*> candidates;
        for (auto block : func->getBlocks())
            for (auto inst : block->getChildren())
            {
                // Every select may choose between resources.
                if (as<IRSelect>(inst))
                    candidates.add(inst);
                // Function parameters receive resources from callers rather than branch arguments,
                // so only parameters of later blocks are candidates.
                else if (as<IRParam>(inst) && block != func->getFirstBlock())
                    candidates.add(inst);
            }
        while (candidates.getCount())
        {
            auto candidate = *candidates.begin();
            candidates.remove(candidate);
            changed |=
                specializeResourceSelection(codeGenContext->getTargetReq(), candidate, candidates);
        }
    }
    return changed;
}

bool specializeResourceUsage(IRModule* irModule, CodeGenContext* codeGenContext)
{
    // Simplification after specialization can remove intermediate loads and stores. We
    // therefore apply selection, input, and output specialization until none of those steps
    // changes the module.
    //
    // Output specialization records a function when callers cannot reconstruct its resource return
    // or output parameter. We then try to inline direct calls to that function, simplify, and retry
    // specialization. Once output inlining makes no progress, the D3D resource-input step
    // tries to inline calls whose resource arguments do not have a recognized HLSL form.
    bool result = false;
    for (;;)
    {
        bool changed = true;
        HashSet<IRFunc*> unspecializableFuncs;

        // We first apply selection, input, and output specialization without removing call
        // boundaries. Simplification after each successful round exposes resource values hidden
        // behind temporary loads and stores.
        while (changed)
        {
            changed = false;
            unspecializableFuncs.clear();
            changed |= specializeResourceSelections(codeGenContext, irModule);
            changed |= specializeResourceOutputs(codeGenContext, irModule, unspecializableFuncs);
            changed |= specializeResourceParameters(codeGenContext, irModule);

            if (changed)
            {
                simplifyIR(
                    irModule,
                    codeGenContext->getTargetProgram(),
                    IRSimplificationOptions::getFast(codeGenContext->getTargetProgram()));
            }
            result |= changed;
        }

        // If callers could not reconstruct a resource return or output parameter, we try to inline
        // each direct call to the callee, simplify the result, and retry selection, input, and
        // output specialization.
        if (unspecializableFuncs.getCount() != 0)
        {
            bool inlinedOutputCall = false;
            for (auto func : unspecializableFuncs)
                inlinedOutputCall |= tryInlineDirectCallsOfNonrecursiveFunction(func);

            if (inlinedOutputCall)
            {
                simplifyIR(
                    irModule,
                    codeGenContext->getTargetProgram(),
                    IRSimplificationOptions::getFast(codeGenContext->getTargetProgram()));
                result = true;
                continue;
            }
        }

        // After output inlining makes no progress, we try to inline the D3D calls whose resource
        // inputs cannot be emitted directly as HLSL. We simplify each successful inlining round and
        // then retry selection, input, and output specialization.
        if (tryInlineD3DCallsWithUnsupportedResourceInputs(codeGenContext, irModule))
        {
            simplifyIR(
                irModule,
                codeGenContext->getTargetProgram(),
                IRSimplificationOptions::getFast(codeGenContext->getTargetProgram()));
            result = true;
            continue;
        }

        return result;
    }
}

bool isIllegalGLSLParameterType(IRType* type)
{
    if (auto arrayType = as<IRArrayTypeBase>(type))
        return isIllegalGLSLParameterType(arrayType->getElementType());

    if (as<IRParameterGroupType>(type))
        return true;
    if (as<IRHLSLStructuredBufferTypeBase>(type))
        return true;
    if (as<IRByteAddressBufferTypeBase>(type))
        return true;
    if (as<IRGLSLImageType>(type))
        return true;
    if (isWritableTextureType(type))
        return true;
    if (as<IRSubpassInputType>(type))
        return true;
    if (as<IRMeshOutputType>(type))
        return true;
    if (as<IRHLSLStreamOutputType>(type))
        return true;
    if (as<IRDynamicResourceType>(type))
        return true;
    if (as<IRHLSLInputPatchType>(type))
        return true;
    if (as<IRHLSLOutputPatchType>(type))
        return true;
    return false;
}

bool isIllegalSPIRVParameterType(IRType* type, bool isArray)
{
    if (isIllegalGLSLParameterType(type))
        return true;

    // If we are emitting SPIRV direclty, we need to specialize
    // all Texture types.
    if (as<IRTextureType>(type))
        return true;
    if (isArray)
    {
        if (as<IRSamplerStateTypeBase>(type))
        {
            return true;
        }
    }
    return false;
}

bool isIllegalWGSLParameterType(IRType* type)
{
    return isIllegalGLSLParameterType(type);
}

void legalizeModesOfNonCopyableOpaqueTypedParamsForGLSL(
    IRModule* irModule,
    CodeGenContext* codeGenContext)
{
    SLANG_UNUSED(codeGenContext);

    auto isNonCopyableOpaqueType = [](IRType* type) -> bool
    {
        type = unwrapArray(type);
        if (as<IRRayQueryType>(type))
            return true;
        if (as<IRHitObjectType>(type))
            return true;
        return false;
    };

    for (auto globalInst : irModule->getGlobalInsts())
    {
        // We only care about functions.
        //
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;

        for (auto param : func->getParams())
        {
            // We are only interested in `out` or `inout` parameters.
            //
            auto paramType = param->getDataType();
            auto paramPtrType = as<IROutParamTypeBase>(paramType);
            if (!paramPtrType)
                continue;

            // We are only interested in parameter that use non-copyable
            // opaque types.
            //
            auto paramValueType = paramPtrType->getValueType();
            if (!isNonCopyableOpaqueType(paramValueType))
                continue;

            // Okay, we've got a parameter with a type that needs to be changed.
            //
            // We'll change an `out` or `inout` parameter over to a
            // `borrow in` parameter, so that the levels of indirection
            // in the IR stay consistent, but the downstream logic to
            // emit GLSL won't go adding a modifier to the parameter declaration.
            //
            IRBuilder builder(irModule);
            auto newParamPtrType = builder.getPtrType(
                kIROp_BorrowInParamType,
                paramValueType,
                paramPtrType->getAccessQualifier(),
                paramPtrType->getAddressSpace(),
                paramPtrType->getDataLayout());
            builder.setDataType(param, newParamPtrType);
        }
    }
}

} // namespace Slang
