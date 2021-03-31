// slang-emit.cpp
#include "slang-emit.h"

#include "../core/slang-writer.h"
#include "../core/slang-type-text-util.h"

#include "../compiler-core/slang-name.h"

#include "slang-ir-bind-existentials.h"
#include "slang-ir-byte-address-legalize.h"
#include "slang-ir-collect-global-uniforms.h"
#include "slang-ir-dce.h"
#include "slang-ir-entry-point-uniforms.h"
#include "slang-ir-entry-point-raw-ptr-params.h"
#include "slang-ir-explicit-global-context.h"
#include "slang-ir-explicit-global-init.h"
#include "slang-ir-glsl-legalize.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-link.h"
#include "slang-ir-lower-generics.h"
#include "slang-ir-lower-tuple-types.h"
#include "slang-ir-lower-bit-cast.h"
#include "slang-ir-restructure.h"
#include "slang-ir-restructure-scoping.h"
#include "slang-ir-specialize.h"
#include "slang-ir-specialize-arrays.h"
#include "slang-ir-specialize-resources.h"
#include "slang-ir-ssa.h"
#include "slang-ir-strip-witness-tables.h"
#include "slang-ir-synthesize-active-mask.h"
#include "slang-ir-union.h"
#include "slang-ir-validate.h"
#include "slang-ir-wrap-structured-buffers.h"
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

#include <assert.h>

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
    BackEndCompileRequest* compileRequest,
    IRModule*       irModule,
    char const*     label = nullptr)
{
    if(compileRequest->shouldDumpIR)
    {
        DiagnosticSinkWriter writer(compileRequest->getSink());

        IRDumpOptions options;
        options.sourceManager = compileRequest->getSourceManager();

        dumpIR(irModule, options, label, &writer);
    }
}

struct LinkingAndOptimizationOptions
{
    bool shouldLegalizeExistentialAndResourceTypes = true;
    CLikeSourceEmitter* sourceEmitter = nullptr;
};

Result linkAndOptimizeIR(
    BackEndCompileRequest*                  compileRequest,
    const List<Int>&                        entryPointIndices,
    CodeGenTarget                           target,
    TargetRequest*                          targetRequest,
    LinkingAndOptimizationOptions const&    options,
    LinkedIR&                               outLinkedIR)
{
    auto sink = compileRequest->getSink();
    auto program = compileRequest->getProgram();
    auto targetProgram = program->getTargetProgram(targetRequest);

    auto session = targetRequest->getSession();

    // We start out by performing "linking" at the level of the IR.
    // This step will create a fresh IR module to be used for
    // code generation, and will copy in any IR definitions that
    // the desired entry point requires. Along the way it will
    // resolve references to imported/exported symbols across
    // modules, and also select between the definitions of
    // any "profile-overloaded" symbols.
    //
    outLinkedIR = linkIR(
        compileRequest,
        entryPointIndices,
        target,
        targetProgram);
    auto irModule = outLinkedIR.module;
    auto irEntryPoints = outLinkedIR.entryPoints;

#if 0
    dumpIRIfEnabled(compileRequest, irModule, "LINKED");
#endif

    validateIRModuleIfEnabled(compileRequest, irModule);

    // If the user specified the flag that they want us to dump
    // IR, then do it here, for the target-specific, but
    // un-specialized IR.
    dumpIRIfEnabled(compileRequest, irModule);

    // Replace any global constants with their values.
    //
    replaceGlobalConstants(irModule);
#if 0
    dumpIRIfEnabled(compileRequest, irModule, "GLOBAL CONSTANTS REPLACED");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);


    // When there are top-level existential-type parameters
    // to the shader, we need to take the side-band information
    // on how the existential "slots" were bound to concrete
    // types, and use it to introduce additional explicit
    // shader parameters for those slots, to be wired up to
    // use sites.
    //
    bindExistentialSlots(irModule, sink);
#if 0
    dumpIRIfEnabled(compileRequest, irModule, "EXISTENTIALS BOUND");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

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
    dumpIRIfEnabled(compileRequest, irModule, "GLOBAL UNIFORMS COLLECTED");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

    // Another transformation that needed to wait until we
    // had layout information on parameters is to take uniform
    // parameters of a shader entry point and move them into
    // the global scope instead.
    //
    // TODO: We should skip this step for CUDA targets.
    //
    {
        CollectEntryPointUniformParamsOptions passOptions;
        switch( target )
        {
        case CodeGenTarget::CUDASource:
            break;

        case CodeGenTarget::CPPSource:
            passOptions.alwaysCreateCollectedParam = true;
        default:
            collectEntryPointUniformParams(irModule, passOptions);
        #if 0
            dumpIRIfEnabled(compileRequest, irModule, "ENTRY POINT UNIFORMS COLLECTED");
        #endif
            validateIRModuleIfEnabled(compileRequest, irModule);
            break;
        }
    }

    switch( target )
    {
    default:
        moveEntryPointUniformParamsToGlobalScope(irModule);
    #if 0
        dumpIRIfEnabled(compileRequest, irModule, "ENTRY POINT UNIFORMS MOVED");
    #endif
        validateIRModuleIfEnabled(compileRequest, irModule);
        break;

    case CodeGenTarget::CPPSource:
    case CodeGenTarget::CUDASource:
        break;
    }


    // Desguar any union types, since these will be illegal on
    // various targets.
    //
    desugarUnionTypes(irModule);
#if 0
    dumpIRIfEnabled(compileRequest, irModule, "UNIONS DESUGARED");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

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
    if (!compileRequest->disableSpecialization)
        specializeModule(irModule);

    eliminateDeadCode(irModule);

    // For targets that supports dynamic dispatch, we need to lower the
    // generics / interface types to ordinary functions and types using
    // function pointers.
    dumpIRIfEnabled(compileRequest, irModule, "BEFORE-LOWER-GENERICS");
    lowerGenerics(targetRequest, irModule, sink);
    dumpIRIfEnabled(compileRequest, irModule, "LOWER-GENERICS");

    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;

    lowerTuples(irModule, sink);
    if (sink->getErrorCount() != 0)
        return SLANG_FAIL;

    // TODO(DG): There are multiple DCE steps here, which need to be changed
    //   so that they don't just throw out any non-entry point code
    // Debugging code for IR transformations...
#if 0
    dumpIRIfEnabled(compileRequest, irModule, "SPECIALIZED");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);


    // Specialization can introduce dead code that could trip
    // up downstream passes like type legalization, so we
    // will run a DCE pass to clean up after the specialization.
    //
    // TODO: Are there other cleanup optimizations we should
    // apply at this point?
    //
    eliminateDeadCode(irModule);
#if 0
    dumpIRIfEnabled(compileRequest, irModule, "AFTER DCE");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

    // We don't need the legalize pass for C/C++ based types
    if(options.shouldLegalizeExistentialAndResourceTypes )
//    if (!(sourceLanguage == SourceLanguage::CPP || sourceStyle == SourceLanguage::C))
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
        eliminateDeadCode(irModule);

#if 0
        dumpIRIfEnabled(compileRequest, irModule, "EXISTENTIALS LEGALIZED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

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
        eliminateDeadCode(irModule);

        //  Debugging output of legalization
    #if 0
        dumpIRIfEnabled(compileRequest, irModule, "LEGALIZED");
    #endif
        validateIRModuleIfEnabled(compileRequest, irModule);
    }

    // Once specialization and type legalization have been performed,
    // we should perform some of our basic optimization steps again,
    // to see if we can clean up any temporaries created by legalization.
    // (e.g., things that used to be aggregated might now be split up,
    // so that we can work with the individual fields).
    constructSSA(irModule);

#if 0
    dumpIRIfEnabled(compileRequest, irModule, "AFTER SSA");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

    // After type legalization and subsequent SSA cleanup we expect
    // that any resource types passed to functions are exposed
    // as their own top-level parameters (which might have
    // resource or array-of-...-resource types).
    //
    // Many of our targets place restrictions on how certain
    // resource types can be used, so that having them as
    // function parameters, reults, etc. is invalid.
    // To clean this up, we apply two kinds of specialization:
    //
    // * Specalize call sites based on the actual resources
    //   that a called function will return/output.
    //
    // * Specialize called functions based on teh actual resources
    //   passed ass input at specific call sites.
    //
    // Because the legalization may depend on what target
    // we are compiling for (certain things might be okay
    // for D3D targets that are not okay for Vulkan), we
    // pass down the target request along with the IR.
    //
    specializeResourceOutputs(compileRequest, targetRequest, irModule);
    specializeResourceParameters(compileRequest, targetRequest, irModule);

    // For GLSL targets, we also want to specialize calls to functions that
    // takes array parameters if possible, to avoid performance issues on
    // those platforms.
    if (isKhronosTarget(targetRequest))
    {
        specializeArrayParameters(compileRequest, targetRequest, irModule);
    }

#if 0
    dumpIRIfEnabled(compileRequest, irModule, "AFTER RESOURCE SPECIALIZATION");
#endif

    validateIRModuleIfEnabled(compileRequest, irModule);

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
                dumpIRIfEnabled(compileRequest, irModule, "STRUCTURED BUFFERS WRAPPED");
#endif
                validateIRModuleIfEnabled(compileRequest, irModule);
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
                auto profile = targetRequest->getTargetProfile();
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

        legalizeByteAddressBufferOps(session, targetRequest, irModule, byteAddressBufferOptions);
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
                compileRequest->getSink());

#if 0
            dumpIRIfEnabled(compileRequest, irModule, "AFTER synthesizeActiveMask");
#endif
            validateIRModuleIfEnabled(compileRequest, irModule);

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
    {
        auto glslExtensionTracker = as<GLSLExtensionTracker>(options.sourceEmitter->getExtensionTracker());

        legalizeEntryPointsForGLSL(
            session,
            irModule,
            irEntryPoints,
            compileRequest->getSink(),
            glslExtensionTracker);

#if 0
            dumpIRIfEnabled(compileRequest, irModule, "GLSL LEGALIZED");
#endif
            validateIRModuleIfEnabled(compileRequest, irModule);
    }
    break;

    case CodeGenTarget::CSource:
    case CodeGenTarget::CPPSource:
        {
            legalizeEntryPointVaryingParamsForCPU(irModule, compileRequest->getSink());
        }
        break;

    case CodeGenTarget::CUDASource:
        {
            legalizeEntryPointVaryingParamsForCUDA(irModule, compileRequest->getSink());
        }
        break;

    default:
        break;
    }


    switch( target )
    {
    default:
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
        dumpIRIfEnabled(compileRequest, irModule, "EXPLICIT GLOBAL CONTEXT INTRODUCED");
    #endif
        validateIRModuleIfEnabled(compileRequest, irModule);
        break;
    }

    // TODO: our current dynamic dispatch pass will remove all uses of witness tables.
    // If we are going to support function-pointer based, "real" modular dynamic dispatch,
    // we will need to disable this pass.
    stripWitnessTables(irModule);

#if 0
    dumpIRIfEnabled(compileRequest, irModule, "AFTER STRIP WITNESS TABLES");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

    // The resource-based specialization pass above
    // may create specialized versions of functions, but
    // it does not try to completely eliminate the original
    // functions, so there might still be invalid code in
    // our IR module.
    //
    // To clean up the code, we will apply a fairly general
    // dead-code-elimination (DCE) pass that only retains
    // whatever code is "live."
    //
    eliminateDeadCode(irModule);
#if 0
    dumpIRIfEnabled(compileRequest, irModule, "AFTER DCE");
#endif
    validateIRModuleIfEnabled(compileRequest, irModule);

    // Lower all bit_cast operations on complex types into leaf-level
    // bit_cast on basic types.
    lowerBitCast(targetRequest, irModule);
    eliminateDeadCode(irModule);
    validateIRModuleIfEnabled(compileRequest, irModule);

    return SLANG_OK;
}

void trackGLSLTargetCaps(
    GLSLExtensionTracker*   extensionTracker,
    CapabilitySet const&    caps);

SlangResult emitEntryPointsSourceFromIR(
    BackEndCompileRequest*  compileRequest,
    const List<Int>&        entryPointIndices,
    CodeGenTarget           target,
    TargetRequest*          targetRequest,
    SourceResult&           outSource)
{
    outSource.reset();

    auto sink = compileRequest->getSink();
    auto program = compileRequest->getProgram();

    auto lineDirectiveMode = compileRequest->getLineDirectiveMode();
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

    SourceWriter sourceWriter(compileRequest->getSourceManager(), lineDirectiveMode );

    CLikeSourceEmitter::Desc desc;

    desc.compileRequest = compileRequest;
    desc.target = target;
    // TODO(DG): Can't assume a single entry point stage for multiple entry points
    if (entryPointIndices.getCount() == 1)
    {
        auto entryPoint = program->getEntryPoint(entryPointIndices[0]);
        desc.entryPointStage = entryPoint->getStage();
        desc.effectiveProfile = getEffectiveProfile(entryPoint, targetRequest);
    }
    else
    {
        desc.entryPointStage = Stage::Unknown;
        desc.effectiveProfile = targetRequest->getTargetProfile();
    }
    desc.targetCaps = targetRequest->getTargetCaps();
    desc.sourceWriter = &sourceWriter;

    // Define here, because must be in scope longer than the sourceEmitter, as sourceEmitter might reference
    // items in the linkedIR module
    LinkedIR linkedIR;

    RefPtr<CLikeSourceEmitter> sourceEmitter;
    
    SourceLanguage sourceLanguage = CLikeSourceEmitter::getSourceLanguage(target);
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

    if (!sourceEmitter)
    {
        sink->diagnose(SourceLoc(), Diagnostics::unableToGenerateCodeForTarget, TypeTextUtil::getCompileTargetName(SlangCompileTarget(target)));
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(sourceEmitter->init());

    {
        LinkingAndOptimizationOptions linkingAndOptimizationOptions;

        linkingAndOptimizationOptions.sourceEmitter = sourceEmitter;

        switch( sourceLanguage )
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
            compileRequest,
            entryPointIndices,
            target,
            targetRequest,
            linkingAndOptimizationOptions,
            linkedIR));

        auto irModule = linkedIR.module;

        // After all of the required optimization and legalization
        // passes have been performed, we can emit target code from
        // the IR module.
        //
        // TODO: do we want to emit directly from IR, or translate the
        // IR back into AST for emission?
#if 0
        dumpIR(compileRequest, irModule, "PRE-EMIT");
#endif
        sourceEmitter->emitModule(irModule, sink);
    }

    String code = sourceWriter.getContent();
    sourceWriter.clearContent();

    // Now that we've emitted the code for all the declarations in the file,
    // it is time to stitch together the final output.

    sourceEmitter->emitPreludeDirectives();

    {
        // If there is a prelude emit it
        const auto& prelude = compileRequest->getSession()->getPreludeForLanguage(sourceLanguage);
        if (prelude.getLength() > 0)
        {
            sourceWriter.emit(prelude.getUnownedSlice());
        }
    }

    // There may be global-scope modifiers that we should emit now
    sourceEmitter->emitPreprocessorDirectives();

    RefObject* extensionTracker = sourceEmitter->getExtensionTracker();

    if (auto glslExtensionTracker = as<GLSLExtensionTracker>(extensionTracker))
    {
        trackGLSLTargetCaps(glslExtensionTracker, targetRequest->getTargetCaps());

        StringBuilder builder;
        glslExtensionTracker->appendExtensionRequireLines(builder);
        sourceWriter.emit(builder.getUnownedSlice());
    }

    sourceEmitter->emitLayoutDirectives(targetRequest);

    String prefix = sourceWriter.getContent();
    
    StringBuilder finalResultBuilder;
    finalResultBuilder << prefix;

    finalResultBuilder << code;

    // Write out the result
    outSource.source = finalResultBuilder.ProduceString();
    outSource.extensionTracker = extensionTracker;

    return SLANG_OK;
}

SlangResult emitSPIRVFromIR(
    BackEndCompileRequest*  compileRequest,
    IRModule*               irModule,
    const List<IRFunc*>&    irEntryPoints,
    List<uint8_t>&          spirvOut);

SlangResult emitSPIRVForEntryPointsDirectly(
    BackEndCompileRequest*  compileRequest,
    const List<Int>&        entryPointIndices,
    TargetRequest*          targetRequest,
    List<uint8_t>&          spirvOut)
{
    auto sink = compileRequest->getSink();
    auto program = compileRequest->getProgram();
    auto targetProgram = program->getTargetProgram(targetRequest);
    auto programLayout = targetProgram->getOrCreateLayout(sink);

    RefPtr<EntryPointLayout> entryPointLayout = programLayout->entryPoints[entryPointIndices[0]];

    // Outside because we want to keep IR in scope whilst we are processing emits
    LinkedIR linkedIR;
    LinkingAndOptimizationOptions linkingAndOptimizationOptions;
    SLANG_RETURN_ON_FAIL(linkAndOptimizeIR(
        compileRequest,
        entryPointIndices,
        targetRequest->getTarget(),
        targetRequest,
        linkingAndOptimizationOptions,
        linkedIR));

    auto irModule = linkedIR.module;
    auto irEntryPoints = linkedIR.entryPoints;

    emitSPIRVFromIR(
        compileRequest,
        irModule,
        irEntryPoints,
        spirvOut);

    return SLANG_OK;
}



} // namespace Slang
