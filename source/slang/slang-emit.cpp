// slang-emit.cpp

#include "../core/slang-writer.h"
#include "../core/slang-type-text-util.h"

#include "../compiler-core/slang-name.h"

#include "slang-ir-any-value-inference.h"
#include "slang-ir-bind-existentials.h"
#include "slang-ir-byte-address-legalize.h"
#include "slang-ir-collect-global-uniforms.h"
#include "slang-ir-cleanup-void.h"
#include "slang-ir-composite-reg-to-mem.h"
#include "slang-ir-dce.h"
#include "slang-ir-diff-call.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-defunctionalization.h"
#include "slang-ir-dll-export.h"
#include "slang-ir-dll-import.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-eliminate-multilevel-break.h"
#include "slang-ir-entry-point-uniforms.h"
#include "slang-ir-entry-point-raw-ptr-params.h"
#include "slang-ir-explicit-global-context.h"
#include "slang-ir-explicit-global-init.h"
#include "slang-ir-fuse-satcoop.h"
#include "slang-ir-glsl-legalize.h"
#include "slang-ir-insts.h"
#include "slang-ir-inline.h"
#include "slang-ir-legalize-array-return-type.h"
#include "slang-ir-legalize-mesh-outputs.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-link.h"
#include "slang-ir-com-interface.h"
#include "slang-ir-lower-append-consume-structured-buffer.h"
#include "slang-ir-lower-binding-query.h"
#include "slang-ir-lower-generics.h"
#include "slang-ir-lower-glsl-ssbo-types.h"
#include "slang-ir-lower-tuple-types.h"
#include "slang-ir-lower-result-type.h"
#include "slang-ir-lower-optional-type.h"
#include "slang-ir-lower-bit-cast.h"
#include "slang-ir-lower-l-value-cast.h"
#include "slang-ir-lower-size-of.h"
#include "slang-ir-lower-reinterpret.h"
#include "slang-ir-loop-unroll.h"
#include "slang-ir-legalize-vector-types.h"
#include "slang-ir-metadata.h"
#include "slang-ir-optix-entry-point-uniforms.h"
#include "slang-ir-pytorch-cpp-binding.h"
#include "slang-ir-restructure.h"
#include "slang-ir-restructure-scoping.h"
#include "slang-ir-sccp.h"
#include "slang-ir-specialize.h"
#include "slang-ir-specialize-arrays.h"
#include "slang-ir-specialize-buffer-load-arg.h"
#include "slang-ir-specialize-resources.h"
#include "slang-ir-specialize-matrix-layout.h"
#include "slang-ir-ssa.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-strip-cached-dict.h"
#include "slang-ir-strip-witness-tables.h"
#include "slang-ir-synthesize-active-mask.h"
#include "slang-ir-validate.h"
#include "slang-ir-wrap-structured-buffers.h"
#include "slang-ir-liveness.h"
#include "slang-ir-glsl-liveness.h"
#include "slang-ir-translate-glsl-global-var.h"
#include "slang-ir-legalize-uniform-buffer-load.h"
#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir-string-hash.h"
#include "slang-ir-simplify-for-emit.h"
#include "slang-ir-pytorch-cpp-binding.h"
#include "slang-ir-uniformity.h"
#include "slang-ir-vk-invert-y.h"
#include "slang-legalize-types.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-syntax.h"
#include "slang-type-layout.h"
#include "slang-visitor.h"

#include "slang-ir-strip.h"

#include "slang-emit-source-writer.h"

#include "slang-emit-c-like.h"

#include "slang-emit-glsl.h"
#include "slang-emit-hlsl.h"
#include "slang-emit-cpp.h"
#include "slang-emit-cuda.h"
#include "slang-emit-torch.h"

#include "slang-spirv-val.h"
#include "../compiler-core/slang-artifact-desc-util.h"
#include "../compiler-core/slang-artifact-util.h"
#include "../compiler-core/slang-artifact-impl.h"
#include "../compiler-core/slang-artifact-associated-impl.h"

#include "../core/slang-castable.h"
#include "../core/slang-performance-profiler.h"

#include <assert.h>

Slang::String get_slang_cpp_host_prelude();
Slang::String get_slang_torch_prelude();

namespace Slang {

EntryPointLayout* findEntryPointLayout(
    ProgramLayout*          programLayout,
    EntryPoint*             entryPoint)
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

    for( auto entryPointLayout : programLayout->entryPoints )
    {
        if(entryPointLayout->entryPoint.getName() != entryPoint->getName())
            continue;

        // TODO: We need to be careful about this check, since it relies on
        // the profile information in the layout matching that in the request.
        //
        // What we really seem to want here is some dictionary mapping the
        // `EntryPoint` directly to the `EntryPointLayout`, and maybe
        // that is precisely what we should build...
        //
        if(entryPointLayout->profile != entryPoint->getProfile())
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
StructTypeLayout* getScopeStructLayout(
    ScopeLayout*  scopeLayout)
{
    auto scopeTypeLayout = scopeLayout->parametersLayout->typeLayout;

    if( auto constantBufferTypeLayout = as<ParameterGroupTypeLayout>(scopeTypeLayout) )
    {
        scopeTypeLayout = constantBufferTypeLayout->offsetElementTypeLayout;
    }

    if( auto structTypeLayout = as<StructTypeLayout>(scopeTypeLayout) )
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
StructTypeLayout* getGlobalStructLayout(
    ProgramLayout*  programLayout)
{
    return getScopeStructLayout(programLayout);
}

static void dumpIRIfEnabled(
    CodeGenContext* codeGenContext,
    IRModule*       irModule,
    char const*     label = nullptr)
{
    if(codeGenContext->shouldDumpIR())
    {
        DiagnosticSinkWriter writer(codeGenContext->getSink());
        //FILE* f = nullptr;
        //fopen_s(&f, (String("dump-") + label + ".txt").getBuffer(), "wt");
        //FileWriter writer(f, 0);
        dumpIR(irModule, codeGenContext->getIRDumpOptions(), label, codeGenContext->getSourceManager(), &writer);
        //fclose(f);
    }
}

struct LinkingAndOptimizationOptions
{
    bool shouldLegalizeExistentialAndResourceTypes = true;
    CLikeSourceEmitter* sourceEmitter = nullptr;
};

Result linkAndOptimizeIR(
    CodeGenContext*                         codeGenContext,
    LinkingAndOptimizationOptions const&    options,
    LinkedIR&                               outLinkedIR)
{
    SLANG_PROFILE;
    auto session = codeGenContext->getSession();
    auto sink = codeGenContext->getSink();
    auto target = codeGenContext->getTargetFormat();
    auto targetRequest = codeGenContext->getTargetReq();
    auto targetProgram = codeGenContext->getTargetProgram();

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

#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "LINKED");
#endif

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // If the user specified the flag that they want us to dump
    // IR, then do it here, for the target-specific, but
    // un-specialized IR.
    dumpIRIfEnabled(codeGenContext, irModule);

    if(!isKhronosTarget(targetRequest))
        lowerGLSLShaderStorageBufferObjectsToStructuredBuffers(irModule, sink);

    translateGLSLGlobalVar(codeGenContext, irModule);

    // Replace any global constants with their values.
    //
    replaceGlobalConstants(irModule);
#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "GLOBAL CONSTANTS REPLACED");
#endif
    validateIRModuleIfEnabled(codeGenContext, irModule);


    // When there are top-level existential-type parameters
    // to the shader, we need to take the side-band information
    // on how the existential "slots" were bound to concrete
    // types, and use it to introduce additional explicit
    // shader parameters for those slots, to be wired up to
    // use sites.
    //
    bindExistentialSlots(irModule, sink);
#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "EXISTENTIALS BOUND");
#endif
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
    collectGlobalUniformParameters(irModule, outLinkedIR.globalScopeVarLayout);
#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "GLOBAL UNIFORMS COLLECTED");
#endif
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
        switch( target )
        {
        case CodeGenTarget::HostCPPSource:
            break;
        case CodeGenTarget::CUDASource:
            collectOptiXEntryPointUniformParams(irModule);
            #if 0
            dumpIRIfEnabled(codeGenContext, irModule, "OPTIX ENTRY POINT UNIFORMS COLLECTED");
            #endif
            validateIRModuleIfEnabled(codeGenContext, irModule);
            break;

        case CodeGenTarget::CPPSource:
            passOptions.alwaysCreateCollectedParam = true;
            [[fallthrough]];
        default:
            collectEntryPointUniformParams(irModule, passOptions);
        #if 0
            dumpIRIfEnabled(codeGenContext, irModule, "ENTRY POINT UNIFORMS COLLECTED");
        #endif
            validateIRModuleIfEnabled(codeGenContext, irModule);
            break;
        }
    }

    switch( target )
    {
    default:
        moveEntryPointUniformParamsToGlobalScope(irModule);
    #if 0
        dumpIRIfEnabled(codeGenContext, irModule, "ENTRY POINT UNIFORMS MOVED");
    #endif
        validateIRModuleIfEnabled(codeGenContext, irModule);
        break;
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CUDASource:
        break;
    }

    lowerOptionalType(irModule, sink);

    switch (target)
    {
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::HostCPPSource:
    {
        lowerComInterfaces(irModule, artifactDesc.style, sink);
        generateDllImportFuncs(codeGenContext->getTargetProgram(), irModule, sink);
        generateDllExportFuncs(irModule, sink);
        break;
    }
    default: break;
    }

    // Lower `Result<T,E>` types into ordinary struct types.
    lowerResultType(irModule, sink);

#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "UNIONS DESUGARED");
#endif
    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Lower all the LValue implict casts (used for out/inout/ref scenarios)
    lowerLValueCast(targetProgram, irModule);

    simplifyIR(targetProgram, irModule, IRSimplificationOptions::getDefault(), sink);

    if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::ValidateUniformity))
    {
        validateUniformity(irModule, sink);
        if (sink->getErrorCount() != 0)
            return SLANG_FAIL;
    }

    // Fill in default matrix layout into matrix types that left layout unspecified.
    specializeMatrixLayout(codeGenContext->getTargetProgram(), irModule);

    // It's important that this takes place before defunctionalization as we
    // want to be able to easily discover the cooperate and fallback funcitons
    // being passed to saturated_cooperation
    fuseCallsToSaturatedCooperation(irModule);

    // Generate any requested derivative wrappers
    generateDerivativeWrappers(irModule, sink);

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
        dumpIRIfEnabled(codeGenContext, irModule, "BEFORE-SPECIALIZE");
        if (!codeGenContext->isSpecializationDisabled())
            changed |= specializeModule(targetProgram, irModule, codeGenContext->getSink());
        if (codeGenContext->getSink()->getErrorCount() != 0)
            return SLANG_FAIL;
        dumpIRIfEnabled(codeGenContext, irModule, "AFTER-SPECIALIZE");

        applySparseConditionalConstantPropagation(irModule, codeGenContext->getSink());
        eliminateDeadCode(irModule);

        validateIRModuleIfEnabled(codeGenContext, irModule);
    
        // Inline calls to any functions marked with [__unsafeInlineEarly] again,
        // since we may be missing out cases prevented by the functions that we just specialzied.
        performMandatoryEarlyInlining(irModule);

        // Unroll loops.
        if (codeGenContext->getSink()->getErrorCount() == 0)
        {
            if (!unrollLoopsInModule(targetProgram, irModule, codeGenContext->getSink()))
                return SLANG_FAIL;
        }

        // Few of our targets support higher order functions, and
        // we don't have the backend code to emit higher order functions for those
        // which do.
        // Specialize away these parameters
        // TODO: We should implement a proper defunctionalization pass
        changed |= specializeHigherOrderParameters(codeGenContext, irModule);

        dumpIRIfEnabled(codeGenContext, irModule, "BEFORE-AUTODIFF");
        enableIRValidationAtInsert();
        changed |= processAutodiffCalls(targetProgram, irModule, sink);
        disableIRValidationAtInsert();
        dumpIRIfEnabled(codeGenContext, irModule, "AFTER-AUTODIFF");

        if (!changed)
            break;
    }

    finalizeAutoDiffPass(targetProgram, irModule);

    finalizeSpecialization(irModule);

    switch (target)
    {
    case CodeGenTarget::PyTorchCppBinding:
        generatePyTorchCppBinding(irModule, sink);
        handleAutoBindNames(irModule);
        break;
    case CodeGenTarget::CUDASource:
        removeTorchKernels(irModule);
        handleAutoBindNames(irModule);
        break;
    default:
        break;
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
        SLANG_RETURN_ON_FAIL(performStringInlining(irModule, sink));
    }

    lowerReinterpret(targetProgram, irModule, sink);
    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // If we have any witness tables that are marked as `KeepAlive`, 
    // but are not used for dynamic dispatch, unpin them so we don't
    // do unnecessary work to lower them.
    unpinWitnessTables(irModule);

    simplifyIR(targetProgram, irModule, IRSimplificationOptions::getFast(), sink);

    if (!ArtifactDescUtil::isCpuLikeTarget(artifactDesc))
    {
        // We could fail because (perhaps, somehow) end up with getStringHash that the operand is not a string literal
        SLANG_RETURN_ON_FAIL(checkGetStringHashInsts(irModule, sink));
    }

    // For targets that supports dynamic dispatch, we need to lower the
    // generics / interface types to ordinary functions and types using
    // function pointers.
    dumpIRIfEnabled(codeGenContext, irModule, "BEFORE-LOWER-GENERICS");
    lowerGenerics(targetProgram, irModule, sink);
    dumpIRIfEnabled(codeGenContext, irModule, "AFTER-LOWER-GENERICS");

    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;
#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "SPECIALIZED");
#endif
    validateIRModuleIfEnabled(codeGenContext, irModule);

    // Inline calls to any functions marked with [__unsafeInlineEarly] or [ForceInline].
    performForceInlining(irModule);

    // Specialization can introduce dead code that could trip
    // up downstream passes like type legalization, so we
    // will run a DCE pass to clean up after the specialization.
    //
    simplifyIR(targetProgram, irModule, IRSimplificationOptions::getDefault(), sink);

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // On non-HLSL targets, there isn't an implementation of `AppendStructuredBuffer`
    // and `ConsumeStructuredBuffer` types, so we lower them into normal struct types
    // of `RWStructuredBuffer` typed fields now.
    if (target != CodeGenTarget::HLSL)
    {
        lowerAppendConsumeStructuredBuffers(targetProgram, irModule, sink);
    }

    // We don't need the legalize pass for C/C++ based types
    if(options.shouldLegalizeExistentialAndResourceTypes )
    {
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
        legalizeExistentialTypeLayout(
            irModule,
            sink);

#if 0
        dumpIRIfEnabled(codeGenContext, irModule, "EXISTENTIALS LEGALIZED");
#endif
        validateIRModuleIfEnabled(codeGenContext, irModule);

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
        legalizeResourceTypes(
            irModule,
            sink);

        //  Debugging output of legalization
    #if 0
        dumpIRIfEnabled(codeGenContext, irModule, "LEGALIZED");
    #endif
        validateIRModuleIfEnabled(codeGenContext, irModule);
    }
    else
    {
        // On CPU/CUDA targets, we simply elminate any empty types if
        // they are not part of public interface.
        legalizeEmptyTypes(
            irModule,
            sink);
    }

    legalizeVectorTypes(irModule, sink);

    // Once specialization and type legalization have been performed,
    // we should perform some of our basic optimization steps again,
    // to see if we can clean up any temporaries created by legalization.
    // (e.g., things that used to be aggregated might now be split up,
    // so that we can work with the individual fields).
    simplifyIR(targetProgram, irModule, IRSimplificationOptions::getFast(), sink);

#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "AFTER SSA");
#endif
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
    specializeResourceUsage(codeGenContext, irModule);
    specializeFuncsForBufferLoadArgs(codeGenContext, irModule);

    // For GLSL targets, we also want to specialize calls to functions that
    // takes array parameters if possible, to avoid performance issues on
    // those platforms.
    if (isKhronosTarget(targetRequest))
    {
        specializeArrayParameters(codeGenContext, irModule);
    }
    eliminateDeadCode(irModule);

#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "AFTER RESOURCE SPECIALIZATION");
#endif

    validateIRModuleIfEnabled(codeGenContext, irModule);

    // For HLSL (and fxc/dxc) only, we need to "wrap" any
    // structured buffers defined over matrix types so
    // that they instead use an intermediate `struct`.
    // This is required to get those targets to respect
    // the options for matrix layout set via `#pragma`
    // or command-line options.
    //
    switch(target)
    {
    case CodeGenTarget::HLSL:
        {
            wrapStructuredBuffersOfMatrices(irModule);
#if 0
            dumpIRIfEnabled(codeGenContext, irModule, "STRUCTURED BUFFERS WRAPPED");
#endif
            validateIRModuleIfEnabled(codeGenContext, irModule);
        }
        break;

    default:
        break;
    }

    // For all targets, we translate load/store operations
    // of aggregate types from/to byte-address buffers into
    // stores of individual scalar or vector values.
    //
    {
        ByteAddressBufferLegalizationOptions byteAddressBufferOptions;

        // Depending on the target, we may decide to do
        // more aggressive translation that reduces the
        // load/store operations down to invididual scalars
        // (splitting up vector ops).
        //
        switch( target )
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
            // TODO: We should consider having an extended variant of `Load<T>`
            // on byte-address buffers which expresses a programmer's knowledge
            // that the load will have greater alignment than required by D3D.
            // That could either come as an explicit guaranteed-alignment
            // operand, or instead as something like a `Load4Aligned<T>` operation
            // that returns a `vector<4,T>` and assumes `4*sizeof(T)` alignemtn.
            //
            byteAddressBufferOptions.scalarizeVectorLoadStore = true;

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
        }

        // We also need to decide whether to translate
        // any "leaf" load/store operations over to
        // use only unsigned-integer types and then
        // bit-cast, or if we prefer to leave them
        // as load/store of the original type.
        //
        switch( target )
        {
        case CodeGenTarget::HLSL:
            {
                auto profile = codeGenContext->getTargetProgram()->getOptionSet().getProfile();
                if( profile.getFamily() == ProfileFamily::DX )
                {
                    if(profile.getVersion() <= ProfileVersion::DX_5_0)
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

        legalizeByteAddressBufferOps(session, targetProgram, irModule, byteAddressBufferOptions);
    }

    // For CUDA targets only, we will need to turn operations
    // the implicitly reference the "active mask" into ones
    // that use (and pass around) an explicit mask instead.
    //
    switch(target)
    {
    case CodeGenTarget::CUDASource:
    case CodeGenTarget::PTX:
        {
            synthesizeActiveMask(
                irModule,
                codeGenContext->getSink());

#if 0
            dumpIRIfEnabled(codeGenContext, irModule, "AFTER synthesizeActiveMask");
#endif
            validateIRModuleIfEnabled(codeGenContext, irModule);

        }
        break;

    default:
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
        GLSLExtensionTracker glslExtensionTracker;
        GLSLExtensionTracker* glslExtensionTrackerPtr = options.sourceEmitter
            ? as<GLSLExtensionTracker>(options.sourceEmitter->getExtensionTracker())
            : &glslExtensionTracker;

        legalizeEntryPointsForGLSL(
            session,
            irModule,
            irEntryPoints,
            codeGenContext,
            glslExtensionTrackerPtr);

#if 0
            dumpIRIfEnabled(codeGenContext, irModule, "GLSL LEGALIZED");
#endif
            validateIRModuleIfEnabled(codeGenContext, irModule);
    }
    break;

    case CodeGenTarget::CSource:
    case CodeGenTarget::CPPSource:
        {
            legalizeEntryPointVaryingParamsForCPU(irModule, codeGenContext->getSink());
        }
        break;

    case CodeGenTarget::CUDASource:
        {
            legalizeEntryPointVaryingParamsForCUDA(irModule, codeGenContext->getSink());
        }
        break;

    default:
        break;
    }

    // Legalize `ImageSubscript` and constant buffer loads for GLSL.
    switch (target)
    {
    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        {
            legalizeImageSubscriptForGLSL(irModule);
            legalizeConstantBufferLoadForGLSL(irModule);
            legalizeDispatchMeshPayloadForGLSL(irModule);
        }
        break;
    default:
        break;
    }

    switch( target )
    {
    default:
        break;
    case CodeGenTarget::GLSL:
    case CodeGenTarget::SPIRV:
    case CodeGenTarget::SPIRVAssembly:
        moveGlobalVarInitializationToEntryPoints(irModule);
        break;
    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CUDASource:
        moveGlobalVarInitializationToEntryPoints(irModule);
        introduceExplicitGlobalContext(irModule, target);
        if(target == CodeGenTarget::CPPSource)
        {
            convertEntryPointPtrParamsToRawPtrs(irModule);
        }
    #if 0
        dumpIRIfEnabled(codeGenContext, irModule, "EXPLICIT GLOBAL CONTEXT INTRODUCED");
    #endif
        validateIRModuleIfEnabled(codeGenContext, irModule);
        break;
    }

    stripCachedDictionaries(irModule);

    // TODO: our current dynamic dispatch pass will remove all uses of witness tables.
    // If we are going to support function-pointer based, "real" modular dynamic dispatch,
    // we will need to disable this pass.
    stripWitnessTables(irModule);

#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "AFTER STRIP WITNESS TABLES");
#endif
    validateIRModuleIfEnabled(codeGenContext, irModule);

    // The resource-based specialization pass above
    // may create specialized versions of functions, but
    // it does not try to completely eliminate the original
    // functions, so there might still be invalid code in
    // our IR module.
    //
    // We run DCE pass again to clean things up.
    //
    eliminateDeadCode(irModule);

    if (isKhronosTarget(targetRequest))
    {
        // As a fallback, if the above specialization steps failed to remove resource type parameters, we will
        // inline the functions in question to make sure we can produce valid GLSL.
        performGLSLResourceReturnFunctionInlining(irModule);
    }
#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "AFTER DCE");
#endif
    validateIRModuleIfEnabled(codeGenContext, irModule);

    cleanUpVoidType(irModule);

    // Lower the `getRegisterIndex` and `getRegisterSpace` intrinsics.
    //
    lowerBindingQueries(irModule, sink);

    // For some small improvement in type safety we represent these as opaque
    // structs instead of regular arrays.
    //
    // If any have survived this far, change them back to regular (decorated)
    // arrays that the emitters can deal with.
    legalizeMeshOutputTypes(irModule);

    if (options.shouldLegalizeExistentialAndResourceTypes)
    {
        // We need to lower any types used in a buffer resource (e.g. ContantBuffer or StructuredBuffer) into
        // a simple storage type that has target independent layout based on the kind of buffer resource.
        lowerBufferElementTypeToStorageType(targetProgram, irModule);
    }

    // Rewrite functions that return arrays to return them via `out` parameter,
    // since our target languages doesn't allow returning arrays.
    legalizeArrayReturnType(irModule);

    if (isKhronosTarget(targetRequest) || target == CodeGenTarget::HLSL)
    {
        legalizeUniformBufferLoad(irModule);
        if (targetProgram->getOptionSet().getBoolOption(CompilerOptionName::VulkanInvertY))
            invertYOfPositionOutput(irModule);
    }

    // Lower sizeof/alignof

    lowerSizeOfLike(targetProgram, irModule, sink);

    // Lower all bit_cast operations on complex types into leaf-level
    // bit_cast on basic types.
    lowerBitCast(targetProgram, irModule);

    bool emitSpirvDirectly = targetProgram->getOptionSet().shouldEmitSPIRVDirectly();

    if (isKhronosTarget(targetRequest) && emitSpirvDirectly)
    {
        performIntrinsicFunctionInlining(irModule);
        eliminateDeadCode(irModule);
    }
    eliminateMultiLevelBreak(irModule);

    {
        IRSimplificationOptions simplificationOptions = IRSimplificationOptions::getFast();
        simplificationOptions.cfgOptions.removeTrivialSingleIterationLoops = true;
        simplifyIR(targetProgram, irModule, simplificationOptions, sink);
    }

    // As a late step, we need to take the SSA-form IR and move things *out*
    // of SSA form, by eliminating all "phi nodes" (block parameters) and
    // introducing explicit temporaries instead. Doing this at the IR level
    // means that subsequent emit logic doesn't need to contend with the
    // complexities of blocks with parameters.
    //
    {
        // Get the liveness mode.
        const LivenessMode livenessMode = codeGenContext->shouldTrackLiveness() ? LivenessMode::Enabled : LivenessMode::Disabled;
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
            LivenessUtil::addVariableRangeStarts(irModule, livenessMode);
        }

        // We only want to accumulate locations if liveness tracking is enabled.
        PhiEliminationOptions phiEliminationOptions;
        if (isKhronosTarget(targetRequest) && emitSpirvDirectly)
        {
            phiEliminationOptions.eliminateCompositeTypedPhiOnly = false;
            phiEliminationOptions.useRegisterAllocation = true;
        }
        eliminatePhis(livenessMode, irModule, phiEliminationOptions);
#if 0
        dumpIRIfEnabled(codeGenContext, irModule, "PHIS ELIMINATED");
#endif

        // If liveness is enabled add liveness ranges based on the accumulated liveness locations

        if (isEnabled(livenessMode))
        {
            LivenessUtil::addRangeEnds(irModule, livenessMode);

#if 0
            dumpIRIfEnabled(codeGenContext, irModule, "LIVENESS");
#endif
        }
    }

    // TODO: We need to insert the logic that fixes variable scoping issues
    // here (rather than doing it very late in the emit process), because
    // otherwise the `applyGLSLLiveness()` operation below wouldn't be
    // able to see the live-range information that pass would need to add.
    // For now we are avoiding that problem by simply *not* emitting live-range
    // information when we fix variable scoping later on.

    // Depending on the target, certain things that were represented as
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
            applyGLSLLiveness(irModule);
        }
    }

    // Run a final round of simplifications to clean up unused things after phi-elimination.
    simplifyNonSSAIR(targetProgram, irModule, IRSimplificationOptions::getFast());

    // We include one final step to (optionally) dump the IR and validate
    // it after all of the optimization passes are complete. This should
    // reflect the IR that code is generated from as closely as possible.
    //
#if 0
    dumpIRIfEnabled(codeGenContext, irModule, "OPTIMIZED");
#endif
    validateIRModuleIfEnabled(codeGenContext, irModule);

    auto metadata = new ArtifactPostEmitMetadata;
    outLinkedIR.metadata = metadata;

    collectMetadata(irModule, *metadata);

    outLinkedIR.metadata = metadata;

    return SLANG_OK;
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

    auto lineDirectiveMode = targetProgram->getOptionSet().getEnumOption<LineDirectiveMode>(CompilerOptionName::LineDirectiveMode);
    // To try to make the default behavior reasonable, we will
    // always use C-style line directives (to give the user
    // good source locations on error messages from downstream
    // compilers) *unless* they requested raw GLSL as the
    // output (in which case we want to maximize compatibility
    // with downstream tools).
    if (lineDirectiveMode ==  LineDirectiveMode::Default && targetRequest->getTarget() == CodeGenTarget::GLSL)
    {
        lineDirectiveMode = LineDirectiveMode::GLSL;
    }

    ComPtr<IBoxValue<SourceMap>> sourceMap;

    // If SourceMap is enabled, we create one and associate it with the sourceWriter
    if (lineDirectiveMode == LineDirectiveMode::SourceMap)
    {
        sourceMap = new BoxValue<SourceMap>;
    }

    SourceWriter sourceWriter(sourceManager, lineDirectiveMode, sourceMap );

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

    // Define here, because must be in scope longer than the sourceEmitter, as sourceEmitter might reference
    // items in the linkedIR module
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
            default: break;
        }
        break;
    case CodeGenTarget::PyTorchCppBinding:
        sourceEmitter = new TorchCppSourceEmitter(desc);
        break;
    }

    if (!sourceEmitter)
    {
        sink->diagnose(SourceLoc(), Diagnostics::unableToGenerateCodeForTarget, TypeTextUtil::getCompileTargetName(SlangCompileTarget(target)));
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

        SLANG_RETURN_ON_FAIL(linkAndOptimizeIR(
            this,
            linkingAndOptimizationOptions,
            linkedIR));
        
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

    // Write out the result

    auto artifact = ArtifactUtil::createArtifactForCompileTarget(asExternal(target));
    artifact->addRepresentationUnknown(StringBlob::moveCreate(finalResult));

    ArtifactUtil::addAssociated(artifact, metadata);

    if (sourceMap)
    {
        auto sourceMapArtifact = ArtifactUtil::createArtifact(ArtifactDesc::make(ArtifactKind::Json, ArtifactPayload::SourceMap, ArtifactStyle::None));

        sourceMapArtifact->addRepresentation(sourceMap);

        artifact->addAssociated(sourceMapArtifact);
    }

    outArtifact.swap(artifact);
    return SLANG_OK;
}

SlangResult emitSPIRVFromIR(
    CodeGenContext*         codeGenContext,
    IRModule*               irModule,
    const List<IRFunc*>&    irEntryPoints,
    List<uint8_t>&          spirvOut);

SlangResult emitSPIRVForEntryPointsDirectly(
    CodeGenContext* codeGenContext,
    ComPtr<IArtifact>& outArtifact)
{
    // Outside because we want to keep IR in scope whilst we are processing emits
    LinkedIR linkedIR;
    LinkingAndOptimizationOptions linkingAndOptimizationOptions;
    SLANG_RETURN_ON_FAIL(linkAndOptimizeIR(
        codeGenContext,
        linkingAndOptimizationOptions,
        linkedIR));

    auto irModule = linkedIR.module;
    auto irEntryPoints = linkedIR.entryPoints;

    List<uint8_t> spirv, outSpirv;
    emitSPIRVFromIR(codeGenContext, irModule, irEntryPoints, spirv);

#if 0
    String optErr;
    if (SLANG_FAILED(optimizeSPIRV(spirv, optErr, outSpirv)))
    {
        codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::spirvOptFailed, optErr);
        spirv = _Move(outSpirv);
    }
#endif
    auto artifact = ArtifactUtil::createArtifactForCompileTarget(asExternal(codeGenContext->getTargetFormat()));
    artifact->addRepresentationUnknown(ListBlob::moveCreate(spirv));

#if 0
    // Dump the unoptimized SPIRV after lowering from slang IR -> SPIRV
    String err; String dis;
    disassembleSPIRV(spirv, err, dis);
    printf("%s", dis.begin());
#endif

    IDownstreamCompiler* compiler = codeGenContext->getSession()->getOrLoadDownstreamCompiler(
        PassThroughMode::SpirvOpt, codeGenContext->getSink());
    if (compiler)
    {
        ComPtr<IArtifact> optimizedArtifact;
        DownstreamCompileOptions downstreamOptions;
        downstreamOptions.sourceArtifacts = makeSlice(artifact.readRef(), 1);
        downstreamOptions.targetType = SLANG_SPIRV;
        downstreamOptions.sourceLanguage = SLANG_SOURCE_LANGUAGE_SPIRV;
        switch (codeGenContext->getTargetProgram()->getOptionSet().getEnumOption<OptimizationLevel>(CompilerOptionName::Optimization))
        {
        case OptimizationLevel::None:       downstreamOptions.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::None; break;
        case OptimizationLevel::Default:    downstreamOptions.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::Default;  break;
        case OptimizationLevel::High:       downstreamOptions.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::High;  break;
        case OptimizationLevel::Maximal:    downstreamOptions.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::Maximal;  break;
        default: SLANG_ASSERT(!"Unhandled optimization level"); break;
        }
        auto downstreamStartTime = std::chrono::high_resolution_clock::now();
        if (SLANG_SUCCEEDED(compiler->compile(downstreamOptions, optimizedArtifact.writeRef())))
        {
            artifact = _Move(optimizedArtifact);
        }
        auto downstreamElapsedTime =
            (std::chrono::high_resolution_clock::now() - downstreamStartTime).count() * 0.000000001;
        codeGenContext->getSession()->addDownstreamCompileTime(downstreamElapsedTime);

        SLANG_RETURN_ON_FAIL(passthroughDownstreamDiagnostics(codeGenContext->getSink(), compiler, artifact));
    }

    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);

    outArtifact.swap(artifact);

    return SLANG_OK;
}

} // namespace Slang
