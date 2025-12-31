// slang-emit.cpp

#include "../compiler-core/slang-artifact-associated-impl.h"
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-artifact-impl.h"
#include "../compiler-core/slang-artifact-util.h"
#include "../compiler-core/slang-name.h"
#include "../core/slang-castable.h"
#include "../core/slang-performance-profiler.h"
#include "../core/slang-type-text-util.h"
#include "../core/slang-writer.h"
#include "slang-check-out-of-bound-access.h"
#include "slang-emit-c-like.h"
#include "slang-emit-cpp.h"
#include "slang-emit-cuda.h"
#include "slang-emit-glsl.h"
#include "slang-emit-hlsl.h"
#include "slang-emit-llvm.h"
#include "slang-emit-metal.h"
#include "slang-emit-slang.h"
#include "slang-emit-source-writer.h"
#include "slang-emit-torch.h"
#include "slang-emit-vm.h"
#include "slang-emit-wgsl.h"
#include "slang-ir-any-value-inference.h"
#include "slang-ir-any-value-marshalling.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-bind-existentials.h"
#include "slang-ir-byte-address-legalize.h"
#include "slang-ir-check-recursion.h"
#include "slang-ir-check-shader-parameter-type.h"
#include "slang-ir-check-unsupported-inst.h"
#include "slang-ir-cleanup-void.h"
#include "slang-ir-collect-global-uniforms.h"
#include "slang-ir-com-interface.h"
#include "slang-ir-composite-reg-to-mem.h"
#include "slang-ir-cuda-immutable-load.h"
#include "slang-ir-dce.h"
#include "slang-ir-defer-buffer-load.h"
#include "slang-ir-defunctionalization.h"
#include "slang-ir-detect-uninitialized-resources.h"
#include "slang-ir-diff-call.h"
#include "slang-ir-dll-export.h"
#include "slang-ir-dll-import.h"
#include "slang-ir-early-raytracing-intrinsic-simplification.h"
#include "slang-ir-eliminate-multilevel-break.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-entry-point-decorations.h"
#include "slang-ir-entry-point-raw-ptr-params.h"
#include "slang-ir-entry-point-uniforms.h"
#include "slang-ir-explicit-global-context.h"
#include "slang-ir-explicit-global-init.h"
#include "slang-ir-fix-entrypoint-callsite.h"
#include "slang-ir-float-non-uniform-resource-index.h"
#include "slang-ir-fuse-satcoop.h"
#include "slang-ir-glsl-legalize.h"
#include "slang-ir-glsl-liveness.h"
#include "slang-ir-hlsl-legalize.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-legalize-array-return-type.h"
#include "slang-ir-legalize-binary-operator.h"
#include "slang-ir-legalize-composite-select.h"
#include "slang-ir-legalize-empty-array.h"
#include "slang-ir-legalize-global-values.h"
#include "slang-ir-legalize-image-subscript.h"
#include "slang-ir-legalize-matrix-types.h"
#include "slang-ir-legalize-mesh-outputs.h"
#include "slang-ir-legalize-uniform-buffer-load.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-legalize-vector-types.h"
#include "slang-ir-link.h"
#include "slang-ir-liveness.h"
#include "slang-ir-loop-unroll.h"
#include "slang-ir-lower-append-consume-structured-buffer.h"
#include "slang-ir-lower-binding-query.h"
#include "slang-ir-lower-bit-cast.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-lower-combined-texture-sampler.h"
#include "slang-ir-lower-coopvec.h"
#include "slang-ir-lower-dynamic-dispatch-insts.h"
#include "slang-ir-lower-dynamic-resource-heap.h"
#include "slang-ir-lower-enum-type.h"
#include "slang-ir-lower-glsl-ssbo-types.h"
#include "slang-ir-lower-l-value-cast.h"
#include "slang-ir-lower-optional-type.h"
#include "slang-ir-lower-reinterpret.h"
#include "slang-ir-lower-result-type.h"
#include "slang-ir-lower-tuple-types.h"
#include "slang-ir-metadata.h"
#include "slang-ir-metal-legalize.h"
#include "slang-ir-missing-return.h"
#include "slang-ir-optix-entry-point-uniforms.h"
#include "slang-ir-pytorch-cpp-binding.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-resolve-texture-format.h"
#include "slang-ir-resolve-varying-input-ref.h"
#include "slang-ir-restructure-scoping.h"
#include "slang-ir-restructure.h"
#include "slang-ir-sccp.h"
#include "slang-ir-simplify-for-emit.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-specialize-arrays.h"
#include "slang-ir-specialize-buffer-load-arg.h"
#include "slang-ir-specialize-matrix-layout.h"
#include "slang-ir-specialize-resources.h"
#include "slang-ir-specialize-stage-switch.h"
#include "slang-ir-specialize.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-ssa.h"
#include "slang-ir-string-hash.h"
#include "slang-ir-strip-debug-info.h"
#include "slang-ir-strip-default-construct.h"
#include "slang-ir-strip-legalization-insts.h"
#include "slang-ir-synthesize-active-mask.h"
#include "slang-ir-transform-params-to-constref.h"
#include "slang-ir-translate-global-varying-var.h"
#include "slang-ir-typeflow-specialize.h"
#include "slang-ir-undo-param-copy.h"
#include "slang-ir-uniformity.h"
#include "slang-ir-user-type-hint.h"
#include "slang-ir-validate.h"
#include "slang-ir-variable-scope-correction.h"
#include "slang-ir-vk-invert-y.h"
#include "slang-ir-wgsl-legalize.h"
#include "slang-ir-wrap-cbuffer-element.h"
#include "slang-ir-wrap-structured-buffers.h"
#include "slang-legalize-types.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-pass-wrapper.h"
#include "slang-syntax.h"
#include "slang-type-layout.h"
#include "slang-visitor.h"
#include "slang-vm-bytecode.h"

#include <assert.h>

Slang::String get_slang_cpp_host_prelude();
Slang::String get_slang_torch_prelude();

namespace Slang
{

EntryPointLayout* findEntryPointLayout(ProgramLayout* programLayout, EntryPoint* entryPoint)
{
    // TODO: This function shouldn't need to exist, and it
    // somewhat hampers the capabilities of the compiler (e.g.,
    // it isn't supported to have a single program contain
    // two different "instances" of the same entry point).
    //
    // Code that cares about layouts should be looking up
    // the entry point layout by index on a `ProgramLayout`,
    // knowing that those indices will align with the order
    // of entry points on the `ComponentType` for the program.

    for (auto entryPointLayout : programLayout->entryPoints)
    {
        if (entryPointLayout->entryPoint.getName() != entryPoint->getName())
            continue;

        // TODO: We need to be careful about this check, since it relies on
        // the profile information in the layout matching that in the request.
        //
        // What we really seem to want here is some dictionary mapping the
        // `EntryPoint` directly to the `EntryPointLayout`, and maybe
        // that is precisely what we should build...
        //
        if (entryPointLayout->profile != entryPoint->getProfile())
            continue;

        return entryPointLayout;
    }

    return nullptr;
}

/// Given a layout computed for a scope, get the layout to use when lookup up variables.
///
/// A scope (such as the global scope of a program) groups its
/// parameters into a pseudo-`struct` type for layout purposes,
/// and in some cases that type will in turn be wrapped in a
/// `ConstantBuffer` type to indicate that the parameters needed
/// an implicit constant buffer to be allocated.
///
/// This function "unwraps" the type layout to find the structure
/// type layout that must be stored inside.
///
StructTypeLayout* getScopeStructLayout(ScopeLayout* scopeLayout)
{
    auto scopeTypeLayout = scopeLayout->parametersLayout->typeLayout;

    if (auto constantBufferTypeLayout = as<ParameterGroupTypeLayout>(scopeTypeLayout))
    {
        scopeTypeLayout = constantBufferTypeLayout->offsetElementTypeLayout;
    }

    if (auto structTypeLayout = as<StructTypeLayout>(scopeTypeLayout))
    {
        return structTypeLayout;
    }

    SLANG_UNEXPECTED("uhandled global-scope binding layout");
    return nullptr;
}

/// Given a layout computed for a program, get the layout to use when lookup up variables.
///
/// This is just an alias of `getScopeStructLayout`.
///
StructTypeLayout* getGlobalStructLayout(ProgramLayout* programLayout)
{
    return getScopeStructLayout(programLayout);
}


static void reportCheckpointIntermediates(
    CodeGenContext* codeGenContext,
    DiagnosticSink* sink,
    IRModule* irModule)
{
    // Report checkpointing information
    TargetRequest* targetReq = codeGenContext->getTargetReq();
    SourceManager* sourceManager = sink->getSourceManager();

    SourceWriter typeWriter(sourceManager, LineDirectiveMode::None, nullptr);

    CLikeSourceEmitter::Desc description;
    description.codeGenContext = codeGenContext;
    description.sourceWriter = &typeWriter;

    CPPSourceEmitter emitter(description);

    int nonEmptyStructs = 0;
    for (auto inst : irModule->getGlobalInsts())
    {
        IRStructType* structType = as<IRStructType>(inst);
        if (!structType)
            continue;

        auto checkpointDecoration =
            structType->findDecoration<IRCheckpointIntermediateDecoration>();
        if (!checkpointDecoration)
            continue;

        IRSizeAndAlignment structSize;
        getNaturalSizeAndAlignment(targetReq, structType, &structSize);

        // Reporting happens before empty structs are optimized out
        // and we still want to keep the checkpointing decorations,
        // so we end up needing to check for non-zero-ness
        if (structSize.size == 0)
            continue;

        auto func = checkpointDecoration->getSourceFunction();
        sink->diagnose(
            structType,
            Diagnostics::reportCheckpointIntermediates,
            func,
            structSize.size);
        nonEmptyStructs++;

        for (auto field : structType->getFields())
        {
            IRType* fieldType = field->getFieldType();
            IRSizeAndAlignment fieldSize;
            getNaturalSizeAndAlignment(targetReq, fieldType, &fieldSize);
            if (fieldSize.size == 0)
                continue;

            typeWriter.clearContent();
            emitter.emitType(fieldType);

            sink->diagnose(
                field->sourceLoc,
                field->findDecoration<IRLoopCounterDecoration>()
                    ? Diagnostics::reportCheckpointCounter
                    : Diagnostics::reportCheckpointVariable,
                fieldSize.size,
                typeWriter.getContent());
        }
    }

    if (nonEmptyStructs == 0)
        sink->diagnose(SourceLoc(), Diagnostics::reportCheckpointNone);
}

struct LinkingAndOptimizationOptions
{
    bool shouldLegalizeExistentialAndResourceTypes = true;
    CLikeSourceEmitter* sourceEmitter = nullptr;
};

// Scan the IR module and determine which lowering/legalization passes are needed based
// on the instructions we see.
//
void calcRequiredLoweringPassSet(
    RequiredLoweringPassSet& result,
    CodeGenContext* codeGenContext,
    IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_DebugValue:
    case kIROp_DebugVar:
    case kIROp_DebugLine:
    case kIROp_DebugLocationDecoration:
    case kIROp_DebugSource:
    case kIROp_DebugInlinedAt:
    case kIROp_DebugScope:
    case kIROp_DebugNoScope:
    case kIROp_DebugFunction:
    case kIROp_DebugBuildIdentifier:
        result.debugInfo = true;
        break;
    case kIROp_ResultType:
        result.resultType = true;
        break;
    case kIROp_OptionalType:
        result.optionalType = true;
        break;
    case kIROp_EnumType:
        result.enumType = true;
        break;
    case kIROp_TextureType:
        if (!isKhronosTarget(codeGenContext->getTargetReq()))
        {
            if (auto texType = as<IRTextureType>(inst))
            {
                auto isCombined = texType->getIsCombinedInst();
                if (auto isCombinedVal = as<IRIntLit>(isCombined))
                {
                    if (isCombinedVal->getValue() != 0)
                    {
                        result.combinedTextureSamplers = true;
                    }
                }
                else
                {
                    result.combinedTextureSamplers = true;
                }
            }
        }
        break;
    case kIROp_PseudoPtrType:
    case kIROp_BoundInterfaceType:
    case kIROp_BindExistentialsType:
        result.generics = true;
        result.existentialTypeLayout = true;
        break;
    case kIROp_GetRegisterIndex:
    case kIROp_GetRegisterSpace:
        result.bindingQuery = true;
        break;
    case kIROp_BackwardDifferentiate:
    case kIROp_ForwardDifferentiate:
    case kIROp_MakeDifferentialPairUserCode:
        result.autodiff = true;
        break;
    case kIROp_VerticesType:
    case kIROp_IndicesType:
    case kIROp_PrimitivesType:
        result.meshOutput = true;
        break;
    case kIROp_CreateExistentialObject:
    case kIROp_MakeExistential:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialValue:
    case kIROp_ExtractExistentialWitnessTable:
    case kIROp_WrapExistential:
    case kIROp_LookupWitnessMethod:
        result.generics = true;
        break;
    case kIROp_Specialize:
        {
            auto specInst = as<IRSpecialize>(inst);
            if (!findAnyTargetIntrinsicDecoration(getResolvedInstForDecorations(specInst)))
                result.generics = true;
        }
        break;
    case kIROp_Reinterpret:
        result.reinterpret = true;
        break;
    case kIROp_BitCast:
        result.bitcast = true;
        break;
    case kIROp_AutoPyBindCudaDecoration:
        result.derivativePyBindWrapper = true;
        break;
    case kIROp_Param:
        if (as<IRFuncType>(inst->getDataType()))
            result.higherOrderFunc = true;
        break;
    case kIROp_GlobalInputDecoration:
    case kIROp_GlobalOutputDecoration:
    case kIROp_GetWorkGroupSize:
        result.globalVaryingVar = true;
        break;
    case kIROp_BindExistentialSlotsDecoration:
        result.bindExistential = true;
        result.generics = true;
        result.existentialTypeLayout = true;
        break;
    case kIROp_GLSLShaderStorageBufferType:
        result.glslSSBO = true;
        break;
    case kIROp_ByteAddressBufferLoad:
    case kIROp_ByteAddressBufferStore:
    case kIROp_HLSLRWByteAddressBufferType:
    case kIROp_HLSLByteAddressBufferType:
        result.byteAddressBuffer = true;
        break;
    case kIROp_DynamicResourceType:
        result.dynamicResource = true;
        break;
    case kIROp_GetDynamicResourceHeap:
        result.dynamicResourceHeap = true;
        break;
    case kIROp_ResolveVaryingInputRef:
        result.resolveVaryingInputRef = true;
        break;
    case kIROp_GetCurrentStage:
        result.specializeStageSwitch = true;
        break;
    case kIROp_MissingReturn:
        result.missingReturn = true;
        break;
    case kIROp_Select:
        if (!isScalarOrVectorType(inst->getFullType()))
            result.nonVectorCompositeSelect = true;
        break;
    }
    if (!result.generics || !result.existentialTypeLayout)
    {
        // If any instruction has an interface type, we need to run
        // the generics lowering pass.
        auto type = as<IRType>(inst) ? inst : inst->getDataType();
        for (;;)
        {
            if (auto ptrType = as<IRPtrTypeBase>(type))
                type = ptrType->getValueType();
            else
                break;
        }
        if (type && type->getOp() == kIROp_InterfaceType)
        {
            result.generics = true;
            result.existentialTypeLayout = true;
        }
    }
    for (auto child : inst->getDecorationsAndChildren())
    {
        calcRequiredLoweringPassSet(
            codeGenContext->getRequiredLoweringPassSet(),
            codeGenContext,
            child);
    }
}

void diagnoseCallStack(IRInst* inst, DiagnosticSink* sink)
{
    static const int maxDepth = 5;
    for (int i = 0; i < maxDepth; i++)
    {
        auto func = getParentFunc(inst);
        if (!func)
            return;
        bool shouldContinue = false;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (auto call = as<IRCall>(user))
            {
                sink->diagnose(call, Diagnostics::seeCallOfFunc, func);
                inst = call;
                shouldContinue = true;
                break;
            }
        }
        if (!shouldContinue)
            return;
    }
}

bool checkStaticAssert(IRInst* inst, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_StaticAssert:
        {
            IRInst* condi = inst->getOperand(0);
            if (auto condiLit = as<IRBoolLit>(condi))
            {
                if (!condiLit->getValue())
                {
                    IRInst* msg = inst->getOperand(1);
                    if (auto msgLit = as<IRStringLit>(msg))
                    {
                        sink->diagnose(
                            inst,
                            Diagnostics::staticAssertionFailure,
                            msgLit->getStringSlice());
                    }
                    else
                    {
                        sink->diagnose(inst, Diagnostics::staticAssertionFailureWithoutMessage);
                    }
                    diagnoseCallStack(inst, sink);
                }
            }
            else
            {
                sink->diagnose(condi, Diagnostics::staticAssertionConditionNotConstant);
            }

            return true;
        }
    }

    List<IRInst*> removeList;
    for (auto child : inst->getChildren())
    {
        if (checkStaticAssert(child, sink))
            removeList.add(child);
    }
    for (auto child : removeList)
    {
        child->removeAndDeallocate();
    }

    return false;
}

static void unexportNonEmbeddableIR(IRModule* irModule, CodeGenTarget target)
{
    for (auto inst : irModule->getGlobalInsts())
    {
        if (inst->getOp() == kIROp_Func)
        {
            bool remove = false;
            if (target == CodeGenTarget::HLSL)
            {
                // DXIL does not permit HLSLStructureBufferType in exported functions
                // or sadly Matrices (https://github.com/shader-slang/slang/issues/4880)
                auto type = as<IRFuncType>(inst->getFullType());
                auto argCount = type->getOperandCount();
                for (UInt aa = 0; aa < argCount; ++aa)
                {
                    auto operand = type->getOperand(aa);
                    if (operand->getOp() == kIROp_HLSLStructuredBufferType ||
                        operand->getOp() == kIROp_MatrixType)
                    {
                        remove = true;
                        break;
                    }
                }
            }
            else if (target == CodeGenTarget::SPIRV)
            {
                // SPIR-V does not allow exporting entry points
                if (inst->findDecoration<IREntryPointDecoration>())
                {
                    remove = true;
                }
            }
            if (remove)
            {
                if (auto dec = inst->findDecoration<IRPublicDecoration>())
                {
                    dec->removeAndDeallocate();
                }
                if (auto dec = inst->findDecoration<IRDownstreamModuleExportDecoration>())
                {
                    dec->removeAndDeallocate();
                }
            }
        }
    }
}

// Add DenormPreserve and DenormFlushToZero decorations to all entry point functions
static void addDenormalModeDecorations(IRModule* irModule, CodeGenContext* codeGenContext)
{
    auto optionSet = codeGenContext->getTargetProgram()->getOptionSet();

    // Only add decorations if we have floating point denormal handling mode options set
    auto denormalModeFp16 = optionSet.getDenormalModeFp16();
    auto denormalModeFp32 = optionSet.getDenormalModeFp32();
    auto denormalModeFp64 = optionSet.getDenormalModeFp64();

    if (denormalModeFp16 == FloatingPointDenormalMode::Any &&
        denormalModeFp32 == FloatingPointDenormalMode::Any &&
        denormalModeFp64 == FloatingPointDenormalMode::Any)
        return;

    IRBuilder builder(irModule);

    // Apply floating point denormal handling mode decorations to all entry point functions
    for (auto inst : irModule->getGlobalInsts())
    {
        IRFunc* func = nullptr;

        // Check if this is a direct function
        if (auto directFunc = as<IRFunc>(inst))
        {
            func = directFunc;
        }
        // Check if this is a generic that contains an entry point function
        else if (auto generic = as<IRGeneric>(inst))
        {
            if (auto innerFunc = as<IRFunc>(findGenericReturnVal(generic)))
            {
                func = innerFunc;
            }
        }

        if (!func)
            continue;

        // Check if this is an entry point function
        auto entryPoint = func->findDecoration<IREntryPointDecoration>();
        if (!entryPoint)
            continue;

        // Handle FP16 denormal handling mode
        auto width16 = builder.getIntValue(builder.getUIntType(), 16);
        if (denormalModeFp16 == FloatingPointDenormalMode::Preserve)
        {
            builder.addFpDenormalPreserveDecoration(func, width16);
        }
        else if (denormalModeFp16 == FloatingPointDenormalMode::FlushToZero)
        {
            builder.addFpDenormalFlushToZeroDecoration(func, width16);
        }

        // Handle FP32 denormal handling mode
        auto width32 = builder.getIntValue(builder.getUIntType(), 32);
        if (denormalModeFp32 == FloatingPointDenormalMode::Preserve)
        {
            builder.addFpDenormalPreserveDecoration(func, width32);
        }
        else if (denormalModeFp32 == FloatingPointDenormalMode::FlushToZero)
        {
            builder.addFpDenormalFlushToZeroDecoration(func, width32);
        }

        // Handle FP64 denormal handling mode
        auto width64 = builder.getIntValue(builder.getUIntType(), 64);
        if (denormalModeFp64 == FloatingPointDenormalMode::Preserve)
        {
            builder.addFpDenormalPreserveDecoration(func, width64);
        }
        else if (denormalModeFp64 == FloatingPointDenormalMode::FlushToZero)
        {
            builder.addFpDenormalFlushToZeroDecoration(func, width64);
        }
    }
}

// Helper function to convert a 20 byte SHA1 to a hexadecimal string,
// needed for the build identifier instruction.
String getBuildIdentifierString(ComponentType* component)
{
    ComPtr<ISlangBlob> hashBlob;
    component->getEntryPointHash(0, 0, hashBlob.writeRef());

    const uint8_t* data = reinterpret_cast<const uint8_t*>(hashBlob->getBufferPointer());
    size_t size = hashBlob->getBufferSize();
    StringBuilder sb;
    for (size_t i = 0; i < size; ++i)
        sb << StringUtil::makeStringWithFormat("%02x", data[i]);
    return sb.produceString();
}

Result linkAndOptimizeIR(
    CodeGenContext* codeGenContext,
    LinkingAndOptimizationOptions const& options,
    LinkedIR& outLinkedIR)
{
    SLANG_PROFILE;

    // This lambda is here so that we can select the correct overload for our parameters, without it
    // the overload deduction fails for passes which have overloads not taking an IRModule*
#define SLANG_PASS(passFunc, ...)                                                          \
    wrapPass(                                                                              \
        codeGenContext,                                                                    \
        #passFunc,                                                                         \
        [](auto* m, auto&&... a) { return passFunc(m, std::forward<decltype(a)>(a)...); }, \
        irModule,                                                                          \
        ##__VA_ARGS__)

    auto session = codeGenContext->getSession();
    auto sink = codeGenContext->getSink();
    auto target = codeGenContext->getTargetFormat();
    auto targetRequest = codeGenContext->getTargetReq();
    auto targetProgram = codeGenContext->getTargetProgram();
    auto targetCompilerOptions = targetRequest->getOptionSet();

    // Get the artifact desc for the target
    const auto artifactDesc = ArtifactDescUtil::makeDescForCompileTarget(asExternal(target));

    // We start out by performing "linking" at the level of the IR.
    // This step will create a fresh IR module to be used for
    // code generation, and will copy in any IR definitions that
    // the desired entry point requires. Along the way it will
    // resolve references to imported/exported symbols across
    // modules, and also select between the definitions of
    // any "profile-overloaded" symbols.
    //
    outLinkedIR = linkIR(codeGenContext);
    auto irModule = outLinkedIR.module;
    auto irEntryPoints = outLinkedIR.entryPoints;

    // For now, only emit the debug build identifier if separate debug info is enabled
    // and only if there are targets.
    // TODO: We will ultimately need to change this to always emit the instruction.
    if (targetCompilerOptions.shouldEmitSeparateDebugInfo())
    {
        // Build identifier is a hash of the source code and compile options.
        String buildIdentifier = getBuildIdentifierString(codeGenContext->getProgram());
        int buildIdentifierFlags = 0;
        IRBuilder builder(irModule);
        builder.setInsertInto(irModule->getModuleInst());
        builder.emitDebugBuildIdentifier(buildIdentifier.getUnownedSlice(), buildIdentifierFlags);
    }

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // If the user specified the flag that they want us to dump
    // IR, then do it here, for the target-specific, but
    // un-specialized IR.

    // Scan the IR module and determine which lowering/legalization passes are needed.
    RequiredLoweringPassSet& requiredLoweringPassSet = codeGenContext->getRequiredLoweringPassSet();
    requiredLoweringPassSet = {};
    calcRequiredLoweringPassSet(requiredLoweringPassSet, codeGenContext, irModule->getModuleInst());

    // Debug info is added by the front-end, and therefore needs to be stripped out by targets
    // that opt out of debug info.
    if (requiredLoweringPassSet.debugInfo &&
        (targetCompilerOptions.getIntOption(CompilerOptionName::DebugInformation) ==
         SLANG_DEBUG_INFO_LEVEL_NONE))
        SLANG_PASS(stripDebugInfo);

    if (!isKhronosTarget(targetRequest) && requiredLoweringPassSet.glslSSBO)
        SLANG_PASS(lowerGLSLShaderStorageBufferObjectsToStructuredBuffers, sink);

    if (requiredLoweringPassSet.globalVaryingVar)
        SLANG_PASS(translateGlobalVaryingVar, codeGenContext);

    if (requiredLoweringPassSet.resolveVaryingInputRef)
        SLANG_PASS(resolveVaryingInputRef);

    SLANG_PASS(fixEntryPointCallsites);

    // Replace any global constants with their values.
    //
    SLANG_PASS(replaceGlobalConstants);
    validateIRModuleIfEnabled(codeGenContext, irModule);


    // When there are top-level existential-type parameters
    // to the shader, we need to take the side-band information
    // on how the existential "slots" were bound to concrete
    // types, and use it to introduce additional explicit
    // shader parameters for those slots, to be wired up to
    // use sites.
    //
    if (requiredLoweringPassSet.bindExistential)
        SLANG_PASS(bindExistentialSlots, sink);
    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Now that we've linked the IR code, any layout/binding
    // information has been attached to shader parameters
    // and entry points. Now we are safe to make transformations
    // that might move code without worrying about losing
    // the connection between a parameter and its layout.

    // One example of a transformation that needs to wait until
    // we have layout information is the step where we collect
    // any global-scope shader parameters with ordinary/uniform
    // type into an aggregate `struct`, and then (optionally)
    // wrap that `struct` up in a constant buffer.
    //
    // This step allows shaders to declare parameters of ordinary
    // type as globals in the input file, while ensuring that
    // downstream passes for graphics APIs like Vulkan and D3D
    // can assume that all ordinary/uniform data is strictly
    // passed using constant buffers.
    //
    SLANG_PASS(collectGlobalUniformParameters, outLinkedIR.globalScopeVarLayout, target);
    validateIRModuleIfEnabled(codeGenContext, irModule);

    SLANG_PASS(checkEntryPointDecorations, target, sink);

    // Add floating point denormal handling mode decorations to entry point functions based on
    // compiler options. This is done post-linking to ensure all entry points from linked
    // modules are processed.
    SLANG_PASS(addDenormalModeDecorations, codeGenContext);
    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Another transformation that needed to wait until we
    // had layout information on parameters is to take uniform
    // parameters of a shader entry point and move them into
    // the global scope instead.
    //
    // TODO: We should skip this step for CUDA targets.
    // (NM): we actually do need to do this step for OptiX based CUDA targets
    //
    {
        CollectEntryPointUniformParamsOptions passOptions;
        passOptions.targetReq = targetRequest;
        switch (target)
        {
        case CodeGenTarget::HostCPPSource:
        case CodeGenTarget::HostVM:
        case CodeGenTarget::HostLLVMIR:
        case CodeGenTarget::HostObjectCode:
        case CodeGenTarget::HostHostCallable:
            break;
        case CodeGenTarget::CUDASource:
        case CodeGenTarget::CUDAHeader:
            SLANG_PASS(collectOptiXEntryPointUniformParams);
            validateIRModuleIfEnabled(codeGenContext, irModule);
            break;

        case CodeGenTarget::CPPSource:
        case CodeGenTarget::CPPHeader:
        case CodeGenTarget::ShaderLLVMIR:
        case CodeGenTarget::ShaderObjectCode:
        case CodeGenTarget::ShaderHostCallable:
            passOptions.alwaysCreateCollectedParam = true;
            [[fallthrough]];
        default:
            SLANG_PASS(collectEntryPointUniformParams, passOptions);
            validateIRModuleIfEnabled(codeGenContext, irModule);
            break;
        }
    }

    switch (target)
    {
    default:
        SLANG_PASS(moveEntryPointUniformParamsToGlobalScope);
        validateIRModuleIfEnabled(codeGenContext, irModule);
        break;
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::HostVM:
    case CodeGenTarget::HostLLVMIR:
    case CodeGenTarget::HostObjectCode:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::ShaderLLVMIR:
    case CodeGenTarget::ShaderObjectCode:
    case CodeGenTarget::ShaderHostCallable:
        break;
    }

    switch (target)
    {
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PyTorchCppBinding:
        break;

    default:
        SLANG_PASS(removeTorchAndCUDAEntryPoints);
        break;
    }

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Lower all the LValue implict casts (used for out/inout/ref scenarios)
    SLANG_PASS(lowerLValueCast, targetProgram);

    // Lower enum types early since enums and enum casts may appear in
    // specialization & not resolving them here would block specialization.
    //
    if (requiredLoweringPassSet.enumType)
        SLANG_PASS(lowerEnumType, sink);

    IRSimplificationOptions defaultIRSimplificationOptions =
        IRSimplificationOptions::getDefault(targetProgram);
    IRSimplificationOptions fastIRSimplificationOptions =
        IRSimplificationOptions::getFast(targetProgram);
    IRDeadCodeEliminationOptions deadCodeEliminationOptions = IRDeadCodeEliminationOptions();
    fastIRSimplificationOptions.minimalOptimization =
        defaultIRSimplificationOptions.minimalOptimization;
    deadCodeEliminationOptions.useFastAnalysis = fastIRSimplificationOptions.minimalOptimization;
    deadCodeEliminationOptions.keepGlobalParamsAlive =
        targetProgram->getOptionSet().getBoolOption(CompilerOptionName::PreserveParameters);

    SLANG_PASS(simplifyIR, targetProgram, defaultIRSimplificationOptions, sink);

    if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::ValidateUniformity))
    {
        SLANG_PASS(validateUniformity, sink);
        if (sink->getErrorCount() != 0)
            return SLANG_FAIL;
    }

    // Fill in default matrix layout into matrix types that left layout unspecified.
    SLANG_PASS(specializeMatrixLayout, targetProgram);

    // It's important that this takes place before defunctionalization as we
    // want to be able to easily discover the cooperate and fallback funcitons
    // being passed to saturated_cooperation
    if (!targetProgram->getOptionSet().shouldPerformMinimumOptimizations())
        SLANG_PASS(fuseCallsToSaturatedCooperation);

    switch (target)
    {
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PyTorchCppBinding:
        {
            // Generate any requested derivative wrappers
            if (requiredLoweringPassSet.derivativePyBindWrapper)
                SLANG_PASS(generateDerivativeWrappers, sink);
            break;
        }
    default:
        break;
    }

    if (requiredLoweringPassSet.autodiff)
    {
        // Generate warnings for potentially incorrect or badly-performing autodiff patterns.
        SLANG_PASS(checkAutodiffPatterns, targetProgram, sink);
    }

    // Next, we need to ensure that the code we emit for
    // the target doesn't contain any operations that would
    // be illegal on the target platform. For example,
    // none of our target supports generics, or interfaces,
    // so we need to specialize those away.
    //
    // Simplification of existential-based and generics-based
    // code may each open up opportunities for the other, so
    // the relevant specialization transformations are handled in a
    // single pass that looks for all simplification opportunities.
    //
    // TODO: We also need to extend this pass so that it will "expose"
    // existential values that are nested inside of other types,
    // so that the simplifications can be applied.
    //
    // TODO: This pass is *also* likely to be the place where we
    // perform specialization of functions based on parameter
    // values that need to be compile-time constants.
    //
    // Specialization passes and auto-diff passes runs in an iterative loop
    // since each pass can enable the other pass to progress further.
    for (;;)
    {
        bool changed = false;
        if (!codeGenContext->isSpecializationDisabled())
        {
            // Pre-autodiff, we will attempt to specialize as much as possible.
            //
            // Note: Lowered dynamic-dispatch code cannot be differentiated correctly due to
            // missing information, so we defer that to after the auto-dff step.
            //
            SpecializationOptions specOptions;
            specOptions.lowerWitnessLookups = false;
            specOptions.reportDynamicDispatchSites =
                codeGenContext->shouldReportDynamicDispatchSites();
            changed |=
                SLANG_PASS(specializeModule, targetProgram, codeGenContext->getSink(), specOptions);
        }

        if (codeGenContext->getSink()->getErrorCount() != 0)
            return SLANG_FAIL;

        if (changed)
        {
            SLANG_PASS(
                applySparseConditionalConstantPropagation,
                targetProgram,
                codeGenContext->getSink());
        }
        validateIRModuleIfEnabled(codeGenContext, irModule);

        // Inline calls to any functions marked with [__unsafeInlineEarly] again,
        // since we may be missing out cases prevented by the functions that we just
        // specialzied.
        SLANG_PASS(performMandatoryEarlyInlining, nullptr);
        SLANG_PASS(eliminateDeadCode, deadCodeEliminationOptions);

        // Unroll loops.
        if (!fastIRSimplificationOptions.minimalOptimization)
        {
            if (codeGenContext->getSink()->getErrorCount() == 0)
            {
                if (!SLANG_PASS(unrollLoopsInModule, targetProgram, codeGenContext->getSink()))
                    return SLANG_FAIL;
            }
        }

        // Few of our targets support higher order functions, and
        // we don't have the backend code to emit higher order functions for those
        // which do.
        // Specialize away these parameters
        // TODO: We should implement a proper defunctionalization pass
        if (requiredLoweringPassSet.higherOrderFunc)
            changed |= SLANG_PASS(specializeHigherOrderParameters, codeGenContext);

        if (requiredLoweringPassSet.autodiff)
        {
            {
                auto validationScope = enableIRValidationScope();
                changed |=
                    SLANG_PASS(processAutodiffCalls, targetProgram, sink, IRAutodiffPassOptions());
            }
        }

        if (!changed)
            break;
    }

    // Report checkpointing information
    if (codeGenContext->shouldReportCheckpointIntermediates())
    {
        SLANG_PASS(simplifyIR, targetProgram, fastIRSimplificationOptions, sink);
        reportCheckpointIntermediates(codeGenContext, sink, irModule);
    }

    // Finalization is always run so AD-related instructions can be removed,
    // even if the AD pass itself is not run.
    //
    SLANG_PASS(finalizeAutoDiffPass, targetProgram);
    SLANG_PASS(eliminateDeadCode, deadCodeEliminationOptions);

    // After auto-diff, we can perform more aggressive specialization with dynamic-dispatch
    // lowering.
    //
    if (!codeGenContext->isSpecializationDisabled())
    {
        SpecializationOptions specOptions;
        specOptions.lowerWitnessLookups = true;
        specOptions.reportDynamicDispatchSites = codeGenContext->shouldReportDynamicDispatchSites();
        SLANG_PASS(specializeModule, targetProgram, codeGenContext->getSink(), specOptions);
    }

    SLANG_PASS(finalizeSpecialization);

    // Lower `Result<T,E>` types into ordinary struct types. This must happen
    // after specialization, since otherwise incompatible copies of the lowered
    // result structure are generated.
    if (requiredLoweringPassSet.resultType)
        SLANG_PASS(lowerResultType, targetProgram, sink);

    if (requiredLoweringPassSet.optionalType)
        SLANG_PASS(lowerOptionalType, sink);

    if (requiredLoweringPassSet.nonVectorCompositeSelect)
    {
        switch (target)
        {
        case CodeGenTarget::HLSL:
            SLANG_PASS(legalizeNonVectorCompositeSelect);
            break;
        default:
            break;
        }
    }

    switch (target)
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::HostCPPSource:
        {
            SLANG_PASS(lowerComInterfaces, artifactDesc.style, sink);
            SLANG_PASS(generateDllImportFuncs, codeGenContext->getTargetProgram(), sink);
            SLANG_PASS(generateDllExportFuncs, sink);
            break;
        }
    default:
        break;
    }

    calcRequiredLoweringPassSet(requiredLoweringPassSet, codeGenContext, irModule->getModuleInst());

    switch (target)
    {
    case CodeGenTarget::PyTorchCppBinding:
        SLANG_PASS(generateHostFunctionsForAutoBindCuda, sink);
        SLANG_PASS(lowerBuiltinTypesForKernelEntryPoints, sink);
        SLANG_PASS(generatePyTorchCppBinding, sink);
        SLANG_PASS(handleAutoBindNames);
        break;
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
        SLANG_PASS(lowerBuiltinTypesForKernelEntryPoints, sink);
        SLANG_PASS(removeTorchKernels);
        SLANG_PASS(handleAutoBindNames);
        break;
    default:
        break;
    }

    SLANG_PASS(detectUninitializedResources, sink);

    if (codeGenContext->removeAvailableInDownstreamIR)
    {
        SLANG_PASS(removeAvailableInDownstreamModuleDecorations, target);
    }

    if (targetProgram->getOptionSet().shouldRunNonEssentialValidation())
    {
        SLANG_PASS(checkForRecursiveTypes, sink);
        SLANG_PASS(checkForRecursiveFunctions, codeGenContext->getTargetReq(), sink);
        SLANG_PASS(checkForOutOfBoundAccess, sink);

        if (requiredLoweringPassSet.missingReturn)
            SLANG_PASS(checkForMissingReturns, sink, target, false);

        // For some targets, we are more restrictive about what types are allowed
        // to be used as shader parameters in ConstantBuffer/ParameterBlock.
        // We will check for these restrictions here.
        SLANG_PASS(checkForInvalidShaderParameterType, targetRequest, sink);
    }

    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;

    // If we have a target that is GPU like we use the string hashing mechanism
    // but for that to work we need to inline such that calls (or returns) of strings
    // boil down into getStringHash(stringLiteral)
    if (!ArtifactDescUtil::isCpuLikeTarget(artifactDesc))
    {
        // We could fail because
        // 1) It's not inlinable for some reason (for example if it's recursive)
        SLANG_RETURN_ON_FAIL(SLANG_PASS(performTypeInlining, targetProgram, sink));
    }

    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;

    validateIRModuleIfEnabled(codeGenContext, irModule);

    SLANG_PASS(inferAnyValueSizeWhereNecessary, targetProgram, sink);

    SLANG_PASS(unpinWitnessTables);
    if (!fastIRSimplificationOptions.minimalOptimization)
    {
        SLANG_PASS(simplifyIR, targetProgram, fastIRSimplificationOptions, sink);
    }
    else if (requiredLoweringPassSet.generics)
    {
        SLANG_PASS(eliminateDeadCode, fastIRSimplificationOptions.deadCodeElimOptions);
    }

    // Tagged union type lowering typically generates more reinterpret instructions.
    if (SLANG_PASS(lowerTaggedUnionTypes, sink))
        requiredLoweringPassSet.reinterpret = true;

    SLANG_PASS(lowerUntaggedUnionTypes, targetProgram, sink);

    if (requiredLoweringPassSet.reinterpret)
        SLANG_PASS(lowerReinterpret, targetProgram, sink);

    SLANG_PASS(lowerSequentialIDTagCasts, codeGenContext->getLinkage(), sink);
    SLANG_PASS(lowerTagInsts, sink);
    SLANG_PASS(lowerTagTypes);

    SLANG_PASS(eliminateDeadCode, fastIRSimplificationOptions.deadCodeElimOptions);

    SLANG_PASS(lowerExistentials, targetProgram, sink);

    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;

    if (!ArtifactDescUtil::isCpuLikeTarget(artifactDesc) &&
        targetProgram->getOptionSet().shouldRunNonEssentialValidation())
    {
        // We could fail because (perhaps, somehow) end up with getStringHash that the operand
        // is not a string literal
        SLANG_RETURN_ON_FAIL(SLANG_PASS(checkGetStringHashInsts, sink));
    }

    SLANG_PASS(lowerTuples, sink);

    SLANG_PASS(generateAnyValueMarshallingFunctions, targetProgram);

    // Don't need to run any further target-dependent passes if we are generating code
    // for host vm.
    if (target == CodeGenTarget::HostVM)
    {
        SLANG_PASS(performForceInlining);
        SLANG_PASS(simplifyIR, targetProgram, defaultIRSimplificationOptions, sink);
        return SLANG_OK;
    }

    // After dynamic dispatch logic is resolved into ordinary function calls,
    // we can now run our stage specialization logic.
    if (requiredLoweringPassSet.specializeStageSwitch)
        SLANG_PASS(specializeStageSwitch);
    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;
    validateIRModuleIfEnabled(codeGenContext, irModule);

    switch (target)
    {
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
    case CodeGenTarget::HLSL:
        break;
    case CodeGenTarget::CUDASource:
        {
            auto targetCaps = targetRequest->getTargetCaps();
            if (!targetCaps.implies(CapabilityAtom::optix_coopvec))
                SLANG_PASS(lowerCooperativeVectors, sink);
        }
        break;
    default:
        SLANG_PASS(lowerCooperativeVectors, sink);
    }

    // Inline calls to any functions marked with [__unsafeInlineEarly] or [ForceInline].
    SLANG_PASS(performForceInlining);

    // Specialization can introduce dead code that could trip
    // up downstream passes like type legalization, so we
    // will run a DCE pass to clean up after the specialization.
    //
    if (fastIRSimplificationOptions.minimalOptimization)
    {
        // Since we force-inlined functions, we need to clean up
        // dead-branches that may have been revealed due to operations
        // like inlining allowing us to find dead branches.
        // These must be cleaned since otherwise static_assert's will falsely
        // detect true due to dead branches with static_assert not being removed.
        SLANG_PASS(

            applySparseConditionalConstantPropagation,
            targetProgram,
            sink);
        SLANG_PASS(eliminateDeadCode, deadCodeEliminationOptions);
    }
    else
    {
        SLANG_PASS(simplifyIR, targetProgram, defaultIRSimplificationOptions, sink);
    }

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // On non-HLSL targets, there isn't an implementation of `AppendStructuredBuffer`
    // and `ConsumeStructuredBuffer` types, so we lower them into normal struct types
    // of `RWStructuredBuffer` typed fields now.
    if (target != CodeGenTarget::HLSL)
    {
        SLANG_PASS(lowerAppendConsumeStructuredBuffers, targetProgram, sink);
    }

    switch (target)
    {
    default:
        if (!ArtifactDescUtil::isCpuLikeTarget(artifactDesc))
            break;
        [[fallthrough]];
    case CodeGenTarget::HLSL:
    case CodeGenTarget::Metal:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::MetalLibAssembly:
    case CodeGenTarget::WGSL:
        if (requiredLoweringPassSet.combinedTextureSamplers)
            SLANG_PASS(lowerCombinedTextureSamplers, codeGenContext, sink);
        break;
    }

    if (codeGenContext->getTargetProgram()->getOptionSet().getBoolOption(
            CompilerOptionName::VulkanEmitReflection))
    {
        SLANG_PASS(addUserTypeHintDecorations);
    }

    SLANG_PASS(legalizeEmptyArray, sink);

    // For CUDA targets, always inline global constants to avoid dynamic initialization
    // of __device__ variables rejected by NVRTC. This runs independently of the broader
    // resource/existential type legalization, which remains disabled for CUDA.
    //
    // We also need this pass on the CPU targets in shader mode, as global
    // constants may reference global parameters, which can't be emitted as
    // constants.
    if (target == CodeGenTarget::CUDASource ||
        (isCPUTarget(targetRequest) && isKernelTarget(target)) ||
        options.shouldLegalizeExistentialAndResourceTypes)
    {
        SLANG_PASS(inlineGlobalConstantsForLegalization);
    }

    // We don't need the legalize pass for C/C++ based types
    if (options.shouldLegalizeExistentialAndResourceTypes)
    {
        if (isMetalTarget(targetRequest))
        {
            // Metal is a special target in that we want to legalize constant buffer
            // types as if it is a bindful target, and skip legalizing parameter block
            // types as if it is a bindless target.
            // To achieve this, we want to ensure that all resource typed fields in parameter
            // blocks are translated into descriptor handles first before running the resource
            // type legalization pass for metal, so that type legalization pass won't mess
            // around with them.
            BufferElementTypeLoweringOptions bufferElementTypeLoweringOptions = {};
            bufferElementTypeLoweringOptions.loweringPolicyKind =
                BufferElementTypeLoweringPolicyKind::MetalParameterBlock;
            SLANG_PASS(
                lowerBufferElementTypeToStorageType,
                targetProgram,
                bufferElementTypeLoweringOptions);
        }

        // The Slang language allows interfaces to be used like
        // ordinary types (including placing them in constant
        // buffers and entry-point parameter lists), but then
        // getting them to lay out in a reasonable way requires
        // us to treat fields/variables with interface type
        // *as if* they were pointers to heap-allocated "objects."
        //
        // Specialization will have replaced fields/variables
        // with interface types like `IFoo` with fields/variables
        // with pointer-like types like `ExistentialBox<SomeType>`.
        //
        // We need to legalize these pointer-like types away,
        // which involves two main changes:
        //
        //  1. Any `ExistentialBox<...>` fields need to be moved
        //  out of their enclosing `struct` type, so that the layout
        //  of the enclosing type is computed as if the field had
        //  zero size.
        //
        //  2. Once an `ExistentialBox<X>` has been floated out
        //  of its parent and landed somwhere permanent (e.g., either
        //  a dedicated variable, or a field of constant buffer),
        //  we need to replace it with just an `X`, after which we
        //  will have (more) legal shader code.
        //
        if (requiredLoweringPassSet.existentialTypeLayout)
        {
            SLANG_PASS(legalizeExistentialTypeLayout, targetProgram, sink);
        }

        validateIRModuleIfEnabled(codeGenContext, irModule);

        if (!validateStructuredBufferResourceTypes(irModule, sink, targetRequest))
            return SLANG_FAIL;

        // Many of our target languages and/or downstream compilers
        // don't support `struct` types that have resource-type fields.
        // In order to work around this limitation, we will rewrite the
        // IR so that any structure types with resource-type fields get
        // split into a "tuple" that comprises the ordinary fields (still
        // bundles up as a `struct`) and one element for each resource-type
        // field (recursively).
        //
        // What used to be individual variables/parameters/arguments/etc.
        // then become multiple variables/parameters/arguments/etc.
        //
        SLANG_PASS(legalizeResourceTypes, targetProgram, sink);

        // We also need to legalize empty types for Metal targets.
        switch (target)
        {
        case CodeGenTarget::Metal:
        case CodeGenTarget::MetalLib:
        case CodeGenTarget::MetalLibAssembly:
            SLANG_PASS(legalizeEmptyTypes, targetProgram, sink);
            break;
        }
        //  Debugging output of legalization
        validateIRModuleIfEnabled(codeGenContext, irModule);
    }
    else
    {
        // On CPU/CUDA targets, we simply elminate any empty types if
        // they are not part of public interface.
        SLANG_PASS(legalizeEmptyTypes, targetProgram, sink);
    }

    if (isCPUTargetViaLLVM(targetRequest))
    {
        // The LLVM targets are special in that we always lower all matrices
        // into arrays of vectors, because matrices aren't (yet?) built-in types
        // in LLVM. Due to this, lowerBufferElementTypeToStorageType needs to
        // occur earlier than usual, because otherwise it no longer knows what
        // is a matrix and can't generate proper code for column/row-major
        // matrix loads/stores.
        BufferElementTypeLoweringOptions bufferElementTypeLoweringOptions = {};
        bufferElementTypeLoweringOptions.loweringPolicyKind =
            BufferElementTypeLoweringPolicyKind::LLVM;
        SLANG_PASS(
            lowerBufferElementTypeToStorageType,
            targetProgram,
            bufferElementTypeLoweringOptions);
    }

    SLANG_PASS(legalizeMatrixTypes, targetProgram, sink);

    SLANG_PASS(legalizeVectorTypes, sink);

    // Once specialization and type legalization have been performed,
    // we should perform some of our basic optimization steps again,
    // to see if we can clean up any temporaries created by legalization.
    // (e.g., things that used to be aggregated might now be split up,
    // so that we can work with the individual fields).
    if (fastIRSimplificationOptions.minimalOptimization)
        SLANG_PASS(eliminateDeadCode, deadCodeEliminationOptions);
    else
        SLANG_PASS(simplifyIR, targetProgram, fastIRSimplificationOptions, sink);

    if (requiredLoweringPassSet.dynamicResourceHeap)
        SLANG_PASS(lowerDynamicResourceHeap, targetProgram, sink);

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // After type legalization and subsequent SSA cleanup we expect
    // that any resource types passed to functions are exposed
    // as their own top-level parameters (which might have
    // resource or array-of-...-resource types).
    //
    // Many of our targets place restrictions on how certain
    // resource types can be used, so that having them as
    // function parameters, reults, etc. is invalid.
    // We clean up the usages of resource values here.
    SLANG_PASS(specializeResourceUsage, codeGenContext);

    // Specialize calls to functions with values loaded from an immutable location,
    // so that we directly load the value inside the callee, instead of loading the
    // value outside of the callee and copy it in. This is necessary to avoid copying
    // large values (e.g. arrays) in registers, where most of the elements are not
    // actually used.
    SLANG_PASS(specializeFuncsForBufferLoadArgs, codeGenContext);

    // Push `structuredBufferLoad` to the end of access chain to avoid loading unnecessary data.
    SLANG_PASS(deferBufferLoad, codeGenContext);

    // We also want to specialize calls to functions that
    // takes unsized array parameters if possible.
    SLANG_PASS(specializeArrayParameters, codeGenContext);

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Process `static_assert` after the specialization is done.
    // Some information for `static_assert` is available only after the specialization.
    checkStaticAssert(irModule->getModuleInst(), sink);

    switch (target)
    {
    case CodeGenTarget::HLSL:
        {
            // For HLSL(fxc) only, we need to "wrap" any
            // structured buffers defined over matrix types so
            // that they instead use an intermediate `struct`.
            // This is required to get those targets to respect
            // the options for matrix layout set via `#pragma`
            // or command-line options.
            //
            SLANG_PASS(wrapStructuredBuffersOfMatrices);
            validateIRModuleIfEnabled(codeGenContext, irModule);
        }
        break;
    case CodeGenTarget::Metal:
    case CodeGenTarget::MetalLib:
        // Metal does not allow `ConstantBuffer<StructuredBuffer<T>>`, so we need to create
        // a wrapper struct for the `StructuredBuffer<T>`.
        SLANG_PASS(wrapCBufferElementsForMetal);
        break;
    default:
        break;
    }

    // For all targets, we translate load/store operations
    // of aggregate types from/to byte-address buffers into
    // stores of individual scalar or vector values.
    //
    if (requiredLoweringPassSet.byteAddressBuffer)
    {
        ByteAddressBufferLegalizationOptions byteAddressBufferOptions;

        // Depending on the target, we may decide to do
        // more aggressive translation that reduces the
        // load/store operations down to invididual scalars
        // (splitting up vector ops).
        //
        switch (target)
        {
        default:
            break;

        case CodeGenTarget::GLSL:
        case CodeGenTarget::SPIRV:
        case CodeGenTarget::SPIRVAssembly:
            // For GLSL targets, we want to translate the vector load/store
            // operations into scalar ops. This is in part as a simplification,
            // but it also ensures that our generated code respects the lax
            // alignment rules for D3D byte-address buffers (the base address
            // of a buffer need not be more than 4-byte aligned, and loads
            // of vectors need only be aligned based on their element type).
            //
            // Slang IR supports a variant of `Load<T>` on byte-address buffers
            // that will have greater alignment than required by D3D. The
            // alignment information is inferred from the operation like a
            // `Load4Aligned<T>` that returns a `vector<4,T>` that assumes a
            // `4*sizeof(T)` alignment. We may choose to disable that in favor
            // of byte-address indexing by setting this flag to true.
            byteAddressBufferOptions.scalarizeVectorLoadStore = false;

            // For GLSL targets, there really isn't a low-level concept
            // of a byte-address buffer at all, and the standard "shader storage
            // buffer" (SSBO) feature is a lot closer to an HLSL structured
            // buffer for our purposes.
            //
            // In particular, each SSBO can only have a single element type,
            // so that even with bitcasts we can't have a single buffer declaration
            // (e.g., one with `uint` elements) service all load/store operations
            // (e.g., a `half` value can't be stored atomically if there are
            // `uint` elements, unless we use explicit atomics).
            //
            // In order to simplify things, we will translate byte-address buffer
            // ops to equivalent structured-buffer ops for GLSL targets, where
            // each unique type being loaded/stored yields a different global
            // parameter declaration of the buffer.
            //
            byteAddressBufferOptions.translateToStructuredBufferOps = true;
            break;
        case CodeGenTarget::Metal:
        case CodeGenTarget::MetalLib:
        case CodeGenTarget::MetalLibAssembly:
            byteAddressBufferOptions.scalarizeVectorLoadStore = true;
            byteAddressBufferOptions.treatGetEquivalentStructuredBufferAsGetThis = true;
            byteAddressBufferOptions.translateToStructuredBufferOps = false;
            byteAddressBufferOptions.lowerBasicTypeOps = true;
            break;
        case CodeGenTarget::WGSL:
        case CodeGenTarget::WGSLSPIRV:
        case CodeGenTarget::WGSLSPIRVAssembly:
            byteAddressBufferOptions.scalarizeVectorLoadStore = true;
            byteAddressBufferOptions.treatGetEquivalentStructuredBufferAsGetThis = true;
            byteAddressBufferOptions.translateToStructuredBufferOps = false;
            byteAddressBufferOptions.lowerBasicTypeOps = true;
            byteAddressBufferOptions.useBitCastFromUInt = true;
            break;
        }

        // We also need to decide whether to translate
        // any "leaf" load/store operations over to
        // use only unsigned-integer types and then
        // bit-cast, or if we prefer to leave them
        // as load/store of the original type.
        //
        switch (target)
        {
        case CodeGenTarget::HLSL:
            {
                auto profile = codeGenContext->getTargetProgram()->getOptionSet().getProfile();
                if (profile.getFamily() == ProfileFamily::DX)
                {
                    if (profile.getVersion() <= ProfileVersion::DX_5_0)
                    {
                        // Fxc and earlier dxc versions do not support
                        // a templates `.Load<T>` operation on byte-address
                        // buffers, and instead need us to emit separate
                        // `uint` loads and then bit-cast over to
                        // the correct type.
                        //
                        byteAddressBufferOptions.useBitCastFromUInt = true;
                    }
                }
            }
            break;

        default:
            break;
        }

        SLANG_PASS(
            legalizeByteAddressBufferOps,
            session,
            targetProgram,
            codeGenContext->getSink(),
            byteAddressBufferOptions);
    }

    // For SPIR-V, this function is called elsewhere, so that it can happen after address space
    // specialization
    if (target != CodeGenTarget::SPIRV && target != CodeGenTarget::SPIRVAssembly)
    {
        bool skipFuncParamValidation = true;
        SLANG_PASS(validateAtomicOperations, skipFuncParamValidation, sink);
    }

    // For CUDA targets only, we will need to turn operations
    // the implicitly reference the "active mask" into ones
    // that use (and pass around) an explicit mask instead.
    //
    switch (target)
    {
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
    case CodeGenTarget::PTX:
        {
            SLANG_PASS(synthesizeActiveMask, codeGenContext->getSink());

            validateIRModuleIfEnabled(codeGenContext, irModule);
        }
        break;

    default:
        break;
    }

    switch (target)
    {
    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::WGSL:
        SLANG_PASS(resolveTextureFormat);
        break;
    }

    // For GLSL only, we will need to perform "legalization" of
    // the entry point and any entry-point parameters.
    //
    // TODO: We should consider moving this legalization work
    // as late as possible, so that it doesn't affect how other
    // optimization passes need to work.
    //
    switch (target)
    {
    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        {
            ShaderExtensionTracker glslExtensionTracker;
            ShaderExtensionTracker* glslExtensionTrackerPtr =
                options.sourceEmitter
                    ? as<ShaderExtensionTracker>(options.sourceEmitter->getExtensionTracker())
                    : &glslExtensionTracker;

            SLANG_PASS(
                legalizeEntryPointsForGLSL,
                session,
                irEntryPoints,
                codeGenContext,
                glslExtensionTrackerPtr);

            validateIRModuleIfEnabled(codeGenContext, irModule);
        }
        break;
    case CodeGenTarget::Metal:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::MetalLibAssembly:
        {
            SLANG_PASS(legalizeIRForMetal, targetProgram, sink);
        }
        break;
    case CodeGenTarget::CSource:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::ShaderLLVMIR:
    case CodeGenTarget::ShaderObjectCode:
    case CodeGenTarget::ShaderHostCallable:
        {
            SLANG_PASS(legalizeEntryPointVaryingParamsForCPU, codeGenContext->getSink());
        }
        break;

    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
        {
            SLANG_PASS(legalizeEntryPointVaryingParamsForCUDA, codeGenContext->getSink());
        }
        break;

    case CodeGenTarget::WGSL:
    case CodeGenTarget::WGSLSPIRV:
    case CodeGenTarget::WGSLSPIRVAssembly:
        {
            SLANG_PASS(legalizeIRForWGSL, targetProgram, sink);
        }
        break;

    default:
        break;
    }

    if (!isSPIRV(targetRequest->getTarget()))
    {
        SLANG_PASS(floatNonUniformResourceIndex, NonUniformResourceIndexFloatMode::Textual);
    }

    if (isD3DTarget(targetRequest) || isKhronosTarget(targetRequest) ||
        isWGPUTarget(targetRequest) || isMetalTarget(targetRequest))
        SLANG_PASS(legalizeLogicalAndOr, targetProgram);

    // Legalize non struct parameters that are expected to be structs for HLSL.
    if (isD3DTarget(targetRequest))
        SLANG_PASS(legalizeNonStructParameterToStructForHLSL);

    // Create aliases for all dynamic resource parameters.
    if (requiredLoweringPassSet.dynamicResource && isKhronosTarget(targetRequest))
        SLANG_PASS(legalizeDynamicResourcesForGLSL, codeGenContext);

    // Legalize `ImageSubscript` loads.
    switch (target)
    {
    case CodeGenTarget::MetalLibAssembly:
    case CodeGenTarget::MetalLib:
    case CodeGenTarget::Metal:
    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        {
            SLANG_PASS(legalizeImageSubscript, targetRequest, sink);
        }
        break;
    default:
        break;
    }

    // Legalize constant buffer loads.
    switch (target)
    {
    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        {
            SLANG_PASS(legalizeConstantBufferLoadForGLSL);
            SLANG_PASS(legalizeDispatchMeshPayloadForGLSL);
        }
        break;
    default:
        break;
    }

    switch (target)
    {
    default:
        break;
    case CodeGenTarget::HLSL:
    case CodeGenTarget::GLSL:
    case CodeGenTarget::WGSL:
        SLANG_PASS(moveGlobalVarInitializationToEntryPoints, targetProgram);
        break;
    // For SPIR-V to SROA across 2 entry-points a value must not be a global
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        SLANG_PASS(moveGlobalVarInitializationToEntryPoints, targetProgram);
        if (targetProgram->getOptionSet().getBoolOption(
                CompilerOptionName::EnableExperimentalPasses))
            SLANG_PASS(introduceExplicitGlobalContext, target);
        SLANG_PASS(transformParamsToConstRef, codeGenContext->getSink());
        break;
    case CodeGenTarget::Metal:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CPPHeader:
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::CUDAHeader:
        // For CUDA/OptiX like targets, add our pass to replace inout parameter copies with
        // direct pointers
        SLANG_PASS(undoParameterCopy);
        // Transform struct parameters to use ConstRef for better performance
        if (isCPUTarget(targetRequest) || isCUDATarget(targetRequest) ||
            isMetalTarget(targetRequest))
        {
            SLANG_PASS(transformParamsToConstRef, codeGenContext->getSink());
        }
        validateIRModuleIfEnabled(codeGenContext, irModule);
        [[fallthrough]];
    case CodeGenTarget::ShaderLLVMIR:
    case CodeGenTarget::ShaderObjectCode:
    case CodeGenTarget::ShaderHostCallable:
        SLANG_PASS(moveGlobalVarInitializationToEntryPoints, targetProgram);
        SLANG_PASS(introduceExplicitGlobalContext, target);
        if (target == CodeGenTarget::CPPSource || target == CodeGenTarget::CPPHeader)
        {
            SLANG_PASS(convertEntryPointPtrParamsToRawPtrs);
        }
        validateIRModuleIfEnabled(codeGenContext, irModule);
        break;
    }

    // TODO: our current dynamic dispatch pass will remove all uses of witness tables.
    // If we are going to support function-pointer based, "real" modular dynamic dispatch,
    // we will need to disable this pass.
    SLANG_PASS(stripLegalizationOnlyInstructions);

    switch (target)
    {
    // On targets that don't support default initialization, remove 'raw' default construct
    // insts because our code-gen will not have any way to emit them.
    //
    case CodeGenTarget::SPIRV:
        if (targetProgram->shouldEmitSPIRVDirectly())
            SLANG_PASS(removeRawDefaultConstructors);
        break;
    default:
        break;
    }

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Validate vectors and matrices according to what the target allows
    SLANG_PASS(validateVectorsAndMatrices, sink, targetRequest);

    // The resource-based specialization pass above
    // may create specialized versions of functions, but
    // it does not try to completely eliminate the original
    // functions, so there might still be invalid code in
    // our IR module.
    //
    // We run DCE pass again to clean things up.
    //
    SLANG_PASS(eliminateDeadCode, deadCodeEliminationOptions);

    SLANG_PASS(cleanUpVoidType);

    if (isKhronosTarget(targetRequest))
    {
        // As a fallback, if the above specialization steps failed to remove resource type
        // parameters, we will inline the functions in question to make sure we can produce
        // valid GLSL.
        SLANG_PASS(performGLSLResourceReturnFunctionInlining, targetProgram);
    }
    validateIRModuleIfEnabled(codeGenContext, irModule);


    // Lower the `getRegisterIndex` and `getRegisterSpace` intrinsics.
    //
    if (requiredLoweringPassSet.bindingQuery)
        SLANG_PASS(lowerBindingQueries, sink);

    // For some small improvement in type safety we represent these as opaque
    // structs instead of regular arrays.
    //
    // If any have survived this far, change them back to regular (decorated)
    // arrays that the emitters can deal with.
    if (requiredLoweringPassSet.meshOutput)
        SLANG_PASS(legalizeMeshOutputTypes);


    // Lower all bit_cast operations on complex types into leaf-level
    // bit_cast on basic types.
    if (requiredLoweringPassSet.bitcast)
        SLANG_PASS(lowerBitCast, targetProgram, sink);

    // Rewrite functions that return arrays to return them via `out` parameter,
    // since our target languages doesn't allow returning arrays.
    if (!isMetalTarget(targetRequest) && !isSPIRV(target))
        SLANG_PASS(legalizeArrayReturnType);

    if (isKhronosTarget(targetRequest) || target == CodeGenTarget::HLSL)
    {
        SLANG_PASS(legalizeUniformBufferLoad);
        if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::VulkanInvertY))
            SLANG_PASS(invertYOfPositionOutput);
        if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::VulkanUseDxPositionW))
            SLANG_PASS(rcpWOfPositionInput);
    }

    BufferElementTypeLoweringOptions bufferElementTypeLoweringOptions = {};
    if (isWGPUTarget(targetRequest))
        bufferElementTypeLoweringOptions.loweringPolicyKind =
            BufferElementTypeLoweringPolicyKind::WGSL;
    else if (isKhronosTarget(targetRequest))
        bufferElementTypeLoweringOptions.loweringPolicyKind =
            BufferElementTypeLoweringPolicyKind::KhronosTarget;
    else
        bufferElementTypeLoweringOptions.loweringPolicyKind =
            BufferElementTypeLoweringPolicyKind::Default;
    SLANG_PASS(
        lowerBufferElementTypeToStorageType,
        targetProgram,
        bufferElementTypeLoweringOptions);

    // If we are generating code for glsl or metal, perform address space propagation now.
    // For SPIRV, we will do that during spirv legalization that happens after
    // `linkAndOptimizeIR`.
    if (target == CodeGenTarget::GLSL)
    {
        NoOpInitialAddressSpaceAssigner addrSpaceAssigner;
        SLANG_PASS(specializeAddressSpace, &addrSpaceAssigner);
    }
    else if (isMetalTarget(targetRequest))
    {
        SLANG_PASS(specializeAddressSpaceForMetal);
    }
    else if (isWGPUTarget(targetRequest))
    {
        SLANG_PASS(specializeAddressSpaceForWGSL);
    }

    bool emitSpirvDirectly = targetProgram->shouldEmitSPIRVDirectly();

    if (isKhronosTarget(targetRequest) && emitSpirvDirectly)
    {
        // If we are emitting SPIR-V directly, we should try to specialize function calls
        // whose argument is an access chain into a global variable/buffer directly after
        // lowerBufferElementTypeToStorageType, so that we can be more SPIRV conformant by
        // eliminating the case where an access chain is passed as function argument.
        // This is disallowed by SPIRV rule 2.16.1 when VariablePointer is not declared.
        SLANG_PASS(specializeFuncsForBufferLoadArgs, codeGenContext);
    }

    // If we are generating code for CUDA, we should translate all immutable buffer loads to
    // using `__ldg` intrinsic for improved performance.
    if (isCUDATarget(targetRequest))
    {
        SLANG_PASS(lowerImmutableBufferLoadForCUDA, targetProgram);
    }

    SLANG_PASS(performForceInlining);

    if (emitSpirvDirectly)
    {
        SLANG_PASS(performIntrinsicFunctionInlining);
    }

    SLANG_PASS(eliminateMultiLevelBreak, targetProgram);

    if (!fastIRSimplificationOptions.minimalOptimization)
    {
        IRSimplificationOptions simplificationOptions = fastIRSimplificationOptions;
        simplificationOptions.cfgOptions.removeTrivialSingleIterationLoops = true;
        SLANG_PASS(simplifyIR, targetProgram, simplificationOptions, sink);
    }

    if (isKhronosTarget(targetRequest) && !emitSpirvDirectly)
    {
        SLANG_PASS(legalizeModesOfNonCopyableOpaqueTypedParamsForGLSL, codeGenContext);
    }

    // As a late step, we need to take the SSA-form IR and move things *out*
    // of SSA form, by eliminating all "phi nodes" (block parameters) and
    // introducing explicit temporaries instead. Doing this at the IR level
    // means that subsequent emit logic doesn't need to contend with the
    // complexities of blocks with parameters.
    //
    {
        // Get the liveness mode.
        const LivenessMode livenessMode =
            codeGenContext->shouldTrackLiveness() ? LivenessMode::Enabled : LivenessMode::Disabled;
        //
        // Downstream targets may benefit from having live-range information for
        // local variables, and our IR currently encodes a reasonably good version
        // of that information. At this point we will insert live-range markers
        // for local variables, on when such markers are requested.
        //
        // After this point in optimization, any passes that introduce new
        // temporary variables into the IR module should take responsibility for
        // producing their own live-range information.
        //
        if (isEnabled(livenessMode))
        {
            SLANG_PASS(LivenessUtil::addVariableRangeStarts, livenessMode);
        }

        // We only want to accumulate locations if liveness tracking is enabled.
        PhiEliminationOptions phiEliminationOptions;
        if (isKhronosTarget(targetRequest) && emitSpirvDirectly)
        {
            phiEliminationOptions.eliminateCompositeTypedPhiOnly = false;
            phiEliminationOptions.useRegisterAllocation = true;
        }
        SLANG_PASS(eliminatePhis, livenessMode, phiEliminationOptions);
        // If liveness is enabled add liveness ranges based on the accumulated liveness
        // locations

        if (isEnabled(livenessMode))
        {
            SLANG_PASS(LivenessUtil::addRangeEnds, livenessMode);
        }
    }

    // TODO: We need to insert the logic that fixes variable scoping issues
    // here (rather than doing it very late in the emit process), because
    // otherwise the `applyGLSLLiveness()` operation below wouldn't be
    // able to see the live-range information that pass would need to add.
    // For now we are avoiding that problem by simply *not* emitting live-range
    // information when we fix variable scoping later on.

    // Depending on the target, certain things that were represented ass
    // single IR instructions will need to be emitted with the help of
    // function declaratons in output high-level code.
    //
    // One example of this is the live-range information, which needs
    // to be output to GLSL code that uses a glslang extension for
    // supporting function declarations that map directly to SPIR-V opcodes.
    //
    // We execute a pass here to transform any live-range instructions
    // in the module into function calls, for the targets that require it.
    //
    if (codeGenContext->shouldTrackLiveness())
    {
        if (isKhronosTarget(targetRequest))
        {
            SLANG_PASS(applyGLSLLiveness);
        }
    }

    if (isKhronosTarget(targetRequest) && emitSpirvDirectly)
    {
        SLANG_PASS(replaceLocationIntrinsicsWithRaytracingObject, targetProgram, sink);
    }

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Run a final round of simplifications to clean up unused things after phi-elimination.
    SLANG_PASS(simplifyNonSSAIR, targetProgram, fastIRSimplificationOptions, sink);

    // We include one final step to (optionally) dump the IR and validate
    // it after all of the optimization passes are complete. This should
    // reflect the IR that code is generated from as closely as possible.
    //
    validateIRModuleIfEnabled(codeGenContext, irModule);

    if ((target != CodeGenTarget::SPIRV) && (target != CodeGenTarget::SPIRVAssembly))
    {
        // We need to perform a final pass to ensure that all the
        // variables in the IR module have their scopes set correctly.
        //
        // This is a separate pass because it needs to run after
        // all the other optimization passes have been performed.

        SLANG_PASS(applyVariableScopeCorrection, targetRequest);
        validateIRModuleIfEnabled(codeGenContext, irModule);
    }

    auto metadata = new ArtifactPostEmitMetadata;
    outLinkedIR.metadata = metadata;

    if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::EmbedDownstreamIR))
    {
        SLANG_PASS(unexportNonEmbeddableIR, target);
    }

    SLANG_PASS(collectMetadata, *metadata);

    outLinkedIR.metadata = metadata;

    if (!targetProgram->getOptionSet().shouldPerformMinimumOptimizations())
        SLANG_PASS(checkUnsupportedInst, codeGenContext->getTargetReq(), sink);

    return sink->getErrorCount() == 0 ? SLANG_OK : SLANG_FAIL;

#undef SLANG_PASS
}

SlangResult CodeGenContext::emitEntryPointsSourceFromIR(ComPtr<IArtifact>& outArtifact)
{
    SLANG_PROFILE;

    outArtifact.setNull();

    auto session = getSession();
    auto sink = getSink();
    auto sourceManager = getSourceManager();
    auto target = getTargetFormat();
    auto targetRequest = getTargetReq();
    auto targetProgram = getTargetProgram();

    auto lineDirectiveMode = targetProgram->getOptionSet().getEnumOption<LineDirectiveMode>(
        CompilerOptionName::LineDirectiveMode);
    // We will generally use C-style line directives in order to give the user good
    // source locations on error messages from downstream compilers, but there are
    // a few exceptions.
    if (lineDirectiveMode == LineDirectiveMode::Default)
    {

        switch (targetRequest->getTarget())
        {

        case CodeGenTarget::GLSL:
            // We want to maximize compatibility with downstream tools.
            lineDirectiveMode = LineDirectiveMode::GLSL;
            break;

        case CodeGenTarget::WGSLSPIRVAssembly:
        case CodeGenTarget::WGSLSPIRV:
        case CodeGenTarget::WGSL:
            // WGSL doesn't support line directives.
            // See https://github.com/gpuweb/gpuweb/issues/606.
            lineDirectiveMode = LineDirectiveMode::None;
            break;
        }
    }

    ComPtr<IBoxValue<SourceMap>> sourceMap;

    // If SourceMap is enabled, we create one and associate it with the sourceWriter
    if (lineDirectiveMode == LineDirectiveMode::SourceMap)
    {
        sourceMap = new BoxValue<SourceMap>;
    }

    SourceWriter sourceWriter(sourceManager, lineDirectiveMode, sourceMap);

    CLikeSourceEmitter::Desc desc;

    desc.codeGenContext = this;

    if (getEntryPointCount() == 1)
    {
        auto entryPoint = getEntryPoint(getSingleEntryPointIndex());
        desc.entryPointStage = entryPoint->getStage();
        desc.effectiveProfile = getEffectiveProfile(entryPoint, targetRequest);
    }
    else
    {
        desc.entryPointStage = Stage::Unknown;
        desc.effectiveProfile = targetProgram->getOptionSet().getProfile();
    }
    desc.sourceWriter = &sourceWriter;

    // Define here, because must be in scope longer than the sourceEmitter, as sourceEmitter might
    // reference items in the linkedIR module
    LinkedIR linkedIR;

    RefPtr<CLikeSourceEmitter> sourceEmitter;
    SourceLanguage sourceLanguage = CLikeSourceEmitter::getSourceLanguage(target);

    switch (target)
    {
    default:
        switch (sourceLanguage)
        {
        case SourceLanguage::CPP:
            {
                sourceEmitter = new CPPSourceEmitter(desc);
                break;
            }
        case SourceLanguage::GLSL:
            {
                sourceEmitter = new GLSLSourceEmitter(desc);
                break;
            }
        case SourceLanguage::HLSL:
            {
                sourceEmitter = new HLSLSourceEmitter(desc);
                break;
            }
        case SourceLanguage::CUDA:
            {
                sourceEmitter = new CUDASourceEmitter(desc);
                break;
            }
        case SourceLanguage::Metal:
            {
                sourceEmitter = new MetalSourceEmitter(desc);
                break;
            }
        case SourceLanguage::WGSL:
            {
                sourceEmitter = new WGSLSourceEmitter(desc);
                break;
            }
        default:
            break;
        }
        break;
    case CodeGenTarget::PyTorchCppBinding:
        sourceEmitter = new TorchCppSourceEmitter(desc);
        break;
    }

    if (!sourceEmitter)
    {
        sink->diagnose(
            SourceLoc(),
            Diagnostics::unableToGenerateCodeForTarget,
            TypeTextUtil::getCompileTargetName(SlangCompileTarget(target)));
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(sourceEmitter->init());

    ComPtr<IArtifactPostEmitMetadata> metadata;
    {
        LinkingAndOptimizationOptions linkingAndOptimizationOptions;

        linkingAndOptimizationOptions.sourceEmitter = sourceEmitter;

        switch (sourceLanguage)
        {
        default:
            break;

        case SourceLanguage::CPP:
        case SourceLanguage::C:
        case SourceLanguage::CUDA:
            linkingAndOptimizationOptions.shouldLegalizeExistentialAndResourceTypes = false;
            break;
        }

        SLANG_RETURN_ON_FAIL(linkAndOptimizeIR(this, linkingAndOptimizationOptions, linkedIR));

        auto irModule = linkedIR.module;

        // Perform final simplifications to help emit logic to generate more compact code.
        simplifyForEmit(irModule, targetRequest);

        metadata = linkedIR.metadata;

        // After all of the required optimization and legalization
        // passes have been performed, we can emit target code from
        // the IR module.
        //
        sourceEmitter->emitModule(irModule, sink);
    }

    String code = sourceWriter.getContent();
    sourceWriter.clearContent();

    // Now that we've emitted the code for all the declarations in the file,
    // it is time to stitch together the final output.

    // There may be global-scope modifiers that we should emit now
    // Supress emitting line directives when emitting preprocessor directives since
    // these preprocessor directives may be required to appear in the first line
    // of the output. An example is that the "#version" line in a GLSL source must
    // appear before anything else.
    sourceWriter.supressLineDirective();

    // When emitting front matter we can emit the target-language-specific directives
    // needed to get the default matrix layout to match what was requested
    // for the given target.
    //
    // Note: we do not rely on the defaults for the target language,
    // because a user could take the HLSL/GLSL generated by Slang and pass
    // it to another compiler with non-default options specified on
    // the command line, leading to all kinds of trouble.
    //
    // TODO: We need an approach to "global" layout directives that will work
    // in the presence of multiple modules. If modules A and B were each
    // compiled with different assumptions about how layout is performed,
    // then types/variables defined in those modules should be emitted in
    // a way that is consistent with that layout...

    // Emit any front matter
    sourceEmitter->emitFrontMatter(targetRequest);

    switch (target)
    {
    case CodeGenTarget::PyTorchCppBinding:
        sourceWriter.emit(get_slang_torch_prelude());
        break;
    default:
        if (isHeterogeneousTarget(target))
        {
            sourceWriter.emit(get_slang_cpp_host_prelude());
        }
        else
        {
            // Get the prelude
            String prelude = session->getPreludeForLanguage(sourceLanguage);
            sourceWriter.emit(prelude);
        }
        break;
    }

    // Emit anything that goes before the contents of the code generated for the module
    sourceEmitter->emitPreModule();

    sourceWriter.resumeLineDirective();

    // Get the content built so far from the front matter/prelude/preModule
    // By getting in this way, the content is no longer referenced by the sourceWriter.
    String finalResult = sourceWriter.getContentAndClear();

    // Append the modules output code
    finalResult.append(code);

    finalResult.append(sourceWriter.getContentAndClear());

    // Write out the result

    auto artifact = ArtifactUtil::createArtifactForCompileTarget(asExternal(target));
    artifact->addRepresentationUnknown(StringBlob::moveCreate(finalResult));

    ArtifactUtil::addAssociated(artifact, metadata);

    if (sourceMap)
    {
        auto sourceMapArtifact = ArtifactUtil::createArtifact(ArtifactDesc::make(
            ArtifactKind::Json,
            ArtifactPayload::SourceMap,
            ArtifactStyle::None));

        sourceMapArtifact->addRepresentation(sourceMap);

        artifact->addAssociated(sourceMapArtifact);
    }

    outArtifact.swap(artifact);
    return SLANG_OK;
}

SlangResult emitSPIRVFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    const List<IRFunc*>& irEntryPoints,
    List<uint8_t>& spirvOut);

// Helper class to assist in stepping through the SPIRV instructions.
class SpirvInstructionHelper
{
public:
    // The result id of the OpExtInstImport instruction, eg.g %2 below
    // %2 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
    uint32_t m_nonSemanticDebugInfoExtSetId = 0;

    // The index of the SPIRV words in the blob.
    // The first 5 words are the header as defined by the SPIRV spec.
    enum SpvWordIndex
    {
        SPV_INDEX_MAGIC_NUMBER,
        SPV_INDEX_VERSION_NUMBER,
        SPV_INDEX_GENERATOR_NUMBER,
        SPV_INDEX_BOUND,
        SPV_INDEX_SCHEMA,
        SPV_INDEX_INSTRUCTION_START,
    };

    // An instruction in the SPIRV blob. This points to the first word of the instruction.
    struct SpvInstruction
    {
        SpvInstruction(SpvWord* word)
            : word(word)
        {
        }

        uint16_t getOpCode() const { return word[0] & 0xFFFF; }
        uint16_t getWordCountForInst() const { return (word[0] >> 16) & 0xFFFF; }
        SpvWord getOperand(uint32_t index) const { return word[index + 1]; }

        // Helper function to interpret the instruction as a string and
        // extract the string value.
        String getStringFromInst() const
        {
            String result;
            for (uint32_t i = 1; i < getWordCountForInst(); i++)
            {
                SpvWord op = getOperand(i);
                for (int b = 0; b < 4; ++b)
                {
                    char c = (char)((op >> (b * 8)) & 0xFF);
                    if (c == '\0')
                        return result;
                    result.append(c);
                }
            }
            return result;
        }

        SpvWord* word = nullptr;
    };

    SpirvInstructionHelper() {}

    // Load the SPIRV instructions from the artifact into a data blob that
    // we can read.
    SlangResult loadBlob(ComPtr<IArtifact>& artifact)
    {
        m_nonSemanticDebugInfoExtSetId = 0;
        m_words.clear();
        m_headerWords.clear();

        ComPtr<ISlangBlob> spirvBlob;
        SlangResult res = artifact->loadBlob(ArtifactKeep::Yes, spirvBlob.writeRef());
        if (SLANG_FAILED(res) || !spirvBlob ||
            spirvBlob->getBufferSize() < SPV_INDEX_INSTRUCTION_START * sizeof(SpvWord))
            return SLANG_FAIL;

        // Populate the full array of SPIR-V words.
        m_words.addRange(
            reinterpret_cast<const SpvWord*>(spirvBlob->getBufferPointer()),
            spirvBlob->getBufferSize() / sizeof(SpvWord));

        // Populate the header words. These are the first 5 words of the SPIR-V
        // blob and are treated differently from the rest of the instructions.
        m_headerWords.clear();
        m_headerWords.addRange(m_words.getBuffer(), SPV_INDEX_INSTRUCTION_START);

        return SLANG_OK;
    }

    // Get the header words.
    const List<SpvWord>& getHeaderWords() const { return m_headerWords; }

    Index getWordCount() const { return m_words.getCount(); }

    // Visit all SPIRV instructions (excluding header words), invoking the callback for each
    // instruction. The callback should be a function or lambda with signature: void(const
    // SpvInstruction&).
    template<typename Func>
    void visitInstructions(Func&& callback)
    {
        // Instructions start after the header (first 5 words)
        constexpr size_t kHeaderWordCount =
            static_cast<size_t>(SpvWordIndex::SPV_INDEX_INSTRUCTION_START);
        size_t i = kHeaderWordCount;
        while (i < (size_t)m_words.getCount())
        {
            SpvWord* wordPtr = m_words.getBuffer() + i;
            SpvInstruction inst(wordPtr);
            callback(inst);
            uint16_t wordCount = inst.getWordCountForInst();
            if (wordCount == 0)
                break; // Prevent infinite loop on malformed input
            i += wordCount;
        }
    }

private:
    // The full array of SPIRV words.
    List<SpvWord> m_words;

    // The header words.
    List<SpvWord> m_headerWords;
};

// Helper function that takes an artifact populated with SPIRV instructions
// after the spirv-opt step, and a previously created but empty
// strippedArtifact. The artifact is unmodified, and the strippedArtifact
// will contain all the artifact's instructions except for debug instructions.
static SlangResult stripDbgSpirvFromArtifact(
    ComPtr<IArtifact>& artifact,
    ComPtr<IArtifact>& strippedArtifact)
{
    // Standard debug opcodes to strip out. This mimics the behavior of
    // spirv-opt.
    static const uint16_t debugOpCodeVals[] = {
        SpvOpSourceContinued,
        SpvOpSource,
        SpvOpSourceExtension,
        SpvOpString,
        SpvOpName,
        SpvOpMemberName,
        SpvOpModuleProcessed,
        SpvOpLine,
        SpvOpNoLine};
    // If the instruction is an extended instruction, then we also need
    // to check if the instruction number is for a debug instruction as
    // listed in slang-emit-spirv-ops-debug-info-ext.h
    static const uint32_t debugExtInstVals[] = {
        NonSemanticShaderDebugInfo100DebugCompilationUnit,
        NonSemanticShaderDebugInfo100DebugTypeBasic,
        NonSemanticShaderDebugInfo100DebugTypePointer,
        NonSemanticShaderDebugInfo100DebugTypeQualifier,
        NonSemanticShaderDebugInfo100DebugTypeArray,
        NonSemanticShaderDebugInfo100DebugTypeVector,
        NonSemanticShaderDebugInfo100DebugTypeFunction,
        NonSemanticShaderDebugInfo100DebugTypeComposite,
        NonSemanticShaderDebugInfo100DebugTypeMember,
        NonSemanticShaderDebugInfo100DebugFunction,
        NonSemanticShaderDebugInfo100DebugScope,
        NonSemanticShaderDebugInfo100DebugNoScope,
        NonSemanticShaderDebugInfo100DebugInlinedAt,
        NonSemanticShaderDebugInfo100DebugLocalVariable,
        NonSemanticShaderDebugInfo100DebugGlobalVariable,
        NonSemanticShaderDebugInfo100DebugInlinedVariable,
        NonSemanticShaderDebugInfo100DebugDeclare,
        NonSemanticShaderDebugInfo100DebugValue,
        NonSemanticShaderDebugInfo100DebugExpression,
        NonSemanticShaderDebugInfo100DebugSource,
        NonSemanticShaderDebugInfo100DebugFunctionDefinition,
        NonSemanticShaderDebugInfo100DebugSourceContinued,
        NonSemanticShaderDebugInfo100DebugLine,
        NonSemanticShaderDebugInfo100DebugEntryPoint,
        NonSemanticShaderDebugInfo100DebugTypeMatrix,
    };

    // Hash sets for easier lookup.
    HashSet<uint16_t> debugOpCodes;
    for (auto val : debugOpCodeVals)
        debugOpCodes.add(val);
    HashSet<uint32_t> debugExtInstNumbers;
    for (auto val : debugExtInstVals)
        debugExtInstNumbers.add(val);

    SpirvInstructionHelper spirvInstructionHelper;
    SLANG_RETURN_ON_FAIL(spirvInstructionHelper.loadBlob(artifact));

    const auto& headerWords = spirvInstructionHelper.getHeaderWords();

    List<uint8_t> spirvWordsList;
    if (auto totalWordCapacity = headerWords.getCount() + spirvInstructionHelper.getWordCount())
    {
        const Index byteCapacity = totalWordCapacity * sizeof(SpvWord);
        spirvWordsList.reserve(byteCapacity);
    }

    spirvWordsList.addRange(
        reinterpret_cast<const uint8_t*>(headerWords.getBuffer()),
        headerWords.getCount() * sizeof(SpvWord));

    // First find the DebugBuildIdentifier instruction, and keep track of which string
    // it refers to, this string needs to be kept in the final output.
    SpvWord debugStringId = 0;
    spirvInstructionHelper.visitInstructions(
        [&](const SpirvInstructionHelper::SpvInstruction& inst)
        {
            if (inst.getOpCode() == SpvOpExtInst)
            {
                if (inst.getOperand(3) == NonSemanticShaderDebugInfo100DebugBuildIdentifier)
                {
                    debugStringId = inst.getOperand(4);
                    return;
                }
            }
            else if (inst.getOpCode() == SpvOpExtInstImport)
            {
                // looking for result id of "OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
                auto importName = inst.getStringFromInst();
                if (importName == "NonSemantic.Shader.DebugInfo.100")
                {
                    spirvInstructionHelper.m_nonSemanticDebugInfoExtSetId = inst.getOperand(0);
                }
            }
        });

    // Iterate over the instructions from the artifact and add them to the list
    // only if they are not debug instructions. We also get the debug build hash
    // to use as the filename for the debug spirv file.
    String debugBuildHash;
    spirvInstructionHelper.visitInstructions(
        [&](const SpirvInstructionHelper::SpvInstruction& inst)
        {
            if (debugOpCodes.contains(inst.getOpCode()))
            {
                // We can only strip strings if they are not being used by the
                // DebugBuildIdentifier instruction.
                bool foundDebugString = false;
                if (inst.getOpCode() == SpvOpString && inst.getOperand(0) == debugStringId)
                {
                    debugBuildHash = inst.getStringFromInst();
                    foundDebugString = true;
                }
                if (!foundDebugString)
                {
                    return;
                }
            }
            // Also check if the instruction is an extended instruction containing DebugInfo.
            if (inst.getOpCode() == SpvOpExtInst)
            {
                // Ignore this if the instruction contains DebugInfo and is from the debug import
                if (debugExtInstNumbers.contains(inst.getOperand(3)) &&
                    inst.getOperand(2) == spirvInstructionHelper.m_nonSemanticDebugInfoExtSetId)
                {
                    return;
                }
            }
            // Otherwise this is a non-debug instruction and should be included.
            spirvWordsList.addRange(
                reinterpret_cast<const uint8_t*>(inst.word),
                inst.getWordCountForInst() * sizeof(SpvWord));
        });

    // Create the stripped artifact using the above created instruction list.
    strippedArtifact->addRepresentationUnknown(ListBlob::moveCreate(spirvWordsList));

    // Set the name of the artifact to the debug build hash so it can be used
    // as the filename for the debug spirv file.
    artifact->setName(debugBuildHash.getBuffer());

    return SLANG_OK;
}

static bool shouldRunSPIRVValidation(CodeGenContext* codeGenContext)
{
    auto& optionSet = codeGenContext->getTargetProgram()->getOptionSet();

    // If the user requested to skip validation, or if we are generating an incomplete library
    // containing linkage decorations (Vulkan validation rules doesn't allow this), we skip
    // validation.
    if (optionSet.getBoolOption(CompilerOptionName::SkipSPIRVValidation) ||
        optionSet.getBoolOption(CompilerOptionName::IncompleteLibrary))
        return false;

    // Also check the environment variable SLANG_RUN_SPIRV_VALIDATION.
    // If it is set to "1", we should run validation.
    StringBuilder runSpirvValEnvVar;
    PlatformUtil::getEnvironmentVariable(
        UnownedStringSlice("SLANG_RUN_SPIRV_VALIDATION"),
        runSpirvValEnvVar);
    if (runSpirvValEnvVar.getUnownedSlice() == "1")
    {
        return true;
    }

    return false;
}

// Helper function to create an artifact from IR used internally by
// emitSPIRVForEntryPointsDirectly.
static SlangResult createArtifactFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    List<IRFunc*> irEntryPoints,
    ComPtr<IArtifact>& artifact,
    ComPtr<IArtifact>& dbgArtifact)
{
    List<uint8_t> spirv, outSpirv;
    emitSPIRVFromIR(codeGenContext, irModule, irEntryPoints, spirv);

    auto targetRequest = codeGenContext->getTargetReq();
    auto targetCompilerOptions = targetRequest->getOptionSet();

#if 0
    String optErr;
    if (SLANG_FAILED(optimizeSPIRV(spirv, optErr, outSpirv)))
    {
        codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::spirvOptFailed, optErr);
        spirv = _Move(outSpirv);
    }
#endif

    artifact->addRepresentationUnknown(ListBlob::moveCreate(spirv));

    IDownstreamCompiler* compiler = codeGenContext->getSession()->getOrLoadDownstreamCompiler(
        PassThroughMode::SpirvOpt,
        codeGenContext->getSink());
    if (compiler)
    {
#if 0
        // Dump the unoptimized/unlinked SPIRV after lowering from slang IR -> SPIRV
        compiler->disassemble((uint32_t*)spirv.getBuffer(), int(spirv.getCount() / 4));
#endif

        bool isPrecompilation = codeGenContext->getTargetProgram()->getOptionSet().getBoolOption(
            CompilerOptionName::EmbedDownstreamIR);

        if (!isPrecompilation && !codeGenContext->shouldSkipDownstreamLinking())
        {
            ComPtr<IArtifact> linkedArtifact;

            // collect spirv files
            List<uint32_t*> spirvFiles;
            List<uint32_t> spirvSizes;

            // Start with the SPIR-V we just generated.
            // SPIRV-Tools-link expects the size in 32-bit words
            // whereas the spirv blob size is in bytes.
            spirvFiles.add((uint32_t*)spirv.getBuffer());
            spirvSizes.add(int(spirv.getCount()) / 4);

            // Iterate over all modules in the linkedIR. For each module, if it
            // contains an embedded downstream ir instruction, add it to the list
            // of spirv files.
            auto program = codeGenContext->getProgram();

            program->enumerateIRModules(
                [&](IRModule* irModule)
                {
                    for (auto globalInst : irModule->getModuleInst()->getChildren())
                    {
                        if (auto inst = as<IREmbeddedDownstreamIR>(globalInst))
                        {
                            if (inst->getTarget() == CodeGenTarget::SPIRV)
                            {
                                auto slice = inst->getBlob()->getStringSlice();
                                spirvFiles.add((uint32_t*)slice.begin());
                                spirvSizes.add(int(slice.getLength()) / 4);
                            }
                        }
                    }
                });

            SLANG_ASSERT(int(spirv.getCount()) % 4 == 0);
            SLANG_ASSERT(spirvFiles.getCount() == spirvSizes.getCount());

            if (spirvFiles.getCount() > 1)
            {
                SlangResult linkresult = compiler->link(
                    (const uint32_t**)spirvFiles.getBuffer(),
                    (const uint32_t*)spirvSizes.getBuffer(),
                    (uint32_t)spirvFiles.getCount(),
                    linkedArtifact.writeRef());

                if (linkresult != SLANG_OK)
                {
                    return SLANG_FAIL;
                }

                ComPtr<ISlangBlob> blob;
                linkedArtifact->loadBlob(ArtifactKeep::No, blob.writeRef());
                artifact = _Move(linkedArtifact);
            }
        }

        if (shouldRunSPIRVValidation(codeGenContext))
        {
            if (SLANG_FAILED(
                    compiler->validate((uint32_t*)spirv.getBuffer(), int(spirv.getCount() / 4))))
            {
                compiler->disassemble((uint32_t*)spirv.getBuffer(), int(spirv.getCount() / 4));
                codeGenContext->getSink()->diagnoseWithoutSourceView(
                    SourceLoc{},
                    Diagnostics::spirvValidationFailed);
            }
        }

        ComPtr<IArtifact> optimizedArtifact;
        DownstreamCompileOptions downstreamOptions;
        downstreamOptions.sourceArtifacts = makeSlice(artifact.readRef(), 1);
        downstreamOptions.targetType = SLANG_SPIRV;
        downstreamOptions.sourceLanguage = SLANG_SOURCE_LANGUAGE_SPIRV;
        switch (codeGenContext->getTargetProgram()->getOptionSet().getEnumOption<OptimizationLevel>(
            CompilerOptionName::Optimization))
        {
        case OptimizationLevel::None:
            downstreamOptions.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::None;
            break;
        case OptimizationLevel::Default:
            downstreamOptions.optimizationLevel =
                DownstreamCompileOptions::OptimizationLevel::Default;
            break;
        case OptimizationLevel::High:
            downstreamOptions.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::High;
            break;
        case OptimizationLevel::Maximal:
            downstreamOptions.optimizationLevel =
                DownstreamCompileOptions::OptimizationLevel::Maximal;
            break;
        default:
            SLANG_ASSERT(!"Unhandled optimization level");
            break;
        }
        auto downstreamStartTime = std::chrono::high_resolution_clock::now();
        if (SLANG_SUCCEEDED(compiler->compile(downstreamOptions, optimizedArtifact.writeRef())))
        {
            // Check if we need to output a separate SPIRV file containing debug info. If so
            // then strip all debug instructions from the artifact. The dbgArtifact will still
            // contain all instructions.
            if (targetCompilerOptions.shouldEmitSeparateDebugInfo())
            {
                auto strippedArtifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_SPIRV);
                SLANG_RETURN_ON_FAIL(
                    stripDbgSpirvFromArtifact(optimizedArtifact, strippedArtifact));
                artifact = _Move(strippedArtifact);
                dbgArtifact = _Move(optimizedArtifact);
            }
            else
                artifact = _Move(optimizedArtifact);
        }
        auto downstreamElapsedTime =
            (std::chrono::high_resolution_clock::now() - downstreamStartTime).count() * 0.000000001;
        codeGenContext->getSession()->addDownstreamCompileTime(downstreamElapsedTime);

        SLANG_RETURN_ON_FAIL(
            passthroughDownstreamDiagnostics(codeGenContext->getSink(), compiler, artifact));
    }

    return SLANG_OK;
}

SlangResult emitSPIRVForEntryPointsDirectly(
    CodeGenContext* codeGenContext,
    ComPtr<IArtifact>& outArtifact)
{
    // Outside because we want to keep IR in scope whilst we are processing emits
    LinkedIR linkedIR;
    LinkingAndOptimizationOptions linkingAndOptimizationOptions;
    SLANG_RETURN_ON_FAIL(
        linkAndOptimizeIR(codeGenContext, linkingAndOptimizationOptions, linkedIR));

    auto irModule = linkedIR.module;
    auto irEntryPoints = linkedIR.entryPoints;


    auto targetRequest = codeGenContext->getTargetReq();
    auto targetCompilerOptions = targetRequest->getOptionSet();

    // Create the artifact containing the main SPIRV data, and the debug SPIRV
    // data if requested by the command line arg -separate-debug-info.
    Slang::ComPtr<Slang::IArtifact> dbgArtifact;
    auto artifact =
        ArtifactUtil::createArtifactForCompileTarget(asExternal(codeGenContext->getTargetFormat()));
    SLANG_RETURN_ON_FAIL(
        createArtifactFromIR(codeGenContext, irModule, irEntryPoints, artifact, dbgArtifact));
    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);

    // Associate the debug artifact with the main artifact.
    // EndToEndCompileRequest::generateOutput will read this data
    // and produce a .dbg.spv file for this child artifact.
    if (targetCompilerOptions.shouldEmitSeparateDebugInfo())
    {
        artifact->addAssociated(dbgArtifact);

        auto artifactPostEmitMetadata =
            static_cast<ArtifactPostEmitMetadata*>(linkedIR.metadata.get());
        artifactPostEmitMetadata->addRef();
        artifactPostEmitMetadata->m_debugBuildIdentifier = dbgArtifact->getName();
    }

    outArtifact.swap(artifact);

    return SLANG_OK;
}

SlangResult emitHostVMCode(CodeGenContext* codeGenContext, ComPtr<IArtifact>& outArtifact)
{
    LinkedIR linkedIR;
    LinkingAndOptimizationOptions linkingAndOptimizationOptions;
    SLANG_RETURN_ON_FAIL(
        linkAndOptimizeIR(codeGenContext, linkingAndOptimizationOptions, linkedIR));

    VMByteCodeBuilder byteCode;
    SLANG_RETURN_ON_FAIL(emitVMByteCodeForEntryPoints(codeGenContext, linkedIR, byteCode));

    String slangDeclaration;
    SLANG_RETURN_ON_FAIL(
        emitSlangDeclarationsForEntryPoints(codeGenContext, linkedIR, slangDeclaration));

    slang::SessionDesc sessionDesc = {};
    ComPtr<slang::ISession> slangSession;
    SLANG_RETURN_ON_FAIL(
        codeGenContext->getSession()->createSession(sessionDesc, slangSession.writeRef()));
    auto linkage = static_cast<Linkage*>(slangSession.get());

    ComPtr<ISlangBlob> diagnostics;
    auto module = slangSession->loadModuleFromSource(
        "kernel",
        "kernel.slang",
        StringBlob::create(slangDeclaration),
        diagnostics.writeRef());
    if (!module)
        return SLANG_FAIL;
    RefPtr<Module> newModule = new Module(linkage);
    newModule->setModuleDecl(static_cast<Module*>(module)->getModuleDecl());
    newModule->setIRModule(linkedIR.module);
    newModule->setName("kernels");
    SLANG_RETURN_ON_FAIL(newModule->serialize(byteCode.kernelBlob.writeRef()));

    ComPtr<slang::IBlob> byteCodeBlob;
    SLANG_RETURN_ON_FAIL(byteCode.serialize(byteCodeBlob.writeRef()));

    outArtifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_HOST_VM);
    outArtifact->addRepresentationUnknown(byteCodeBlob);

    return SLANG_OK;
}

SlangResult emitLLVMForEntryPoints(CodeGenContext* codeGenContext, ComPtr<IArtifact>& outArtifact)
{
    auto target = codeGenContext->getTargetFormat();

    ISlangSharedLibrary* library = codeGenContext->getSession()->getOrLoadSlangLLVM();
    if (!library)
    {
        codeGenContext->getSink()->diagnose(
            SourceLoc(),
            Diagnostics::unableToGenerateCodeForTarget,
            TypeTextUtil::getCompileTargetName(SlangCompileTarget(target)));
        return SLANG_FAIL;
    }

    LinkedIR linkedIR;
    LinkingAndOptimizationOptions linkingAndOptimizationOptions;
    linkingAndOptimizationOptions.shouldLegalizeExistentialAndResourceTypes = false;
    SLANG_RETURN_ON_FAIL(
        linkAndOptimizeIR(codeGenContext, linkingAndOptimizationOptions, linkedIR));

    auto irModule = linkedIR.module;

    ComPtr<ISlangBlob> blob;

    switch (target)
    {
    // At this point there should be no difference between host style and shader
    // style from LLVM's perspective: the shader style has already been
    // lowered/legalized into host style.
    case CodeGenTarget::HostObjectCode:
    case CodeGenTarget::ShaderObjectCode:
        {
            IArtifact* artifact = nullptr;
            emitLLVMObjectFromIR(codeGenContext, irModule, &artifact);
            outArtifact = ComPtr<IArtifact>(artifact);
        }
        break;
    case CodeGenTarget::HostLLVMIR:
    case CodeGenTarget::ShaderLLVMIR:
        {
            IArtifact* artifact = nullptr;
            emitLLVMAssemblyFromIR(codeGenContext, irModule, &artifact);
            outArtifact = ComPtr<IArtifact>(artifact);
        }
        break;
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::ShaderHostCallable:
        {
            IArtifact* artifact = nullptr;
            emitLLVMJITFromIR(codeGenContext, irModule, &artifact);
            outArtifact = ComPtr<IArtifact>(artifact);
        }
        break;
    }

    return SLANG_OK;
}

} // namespace Slang
