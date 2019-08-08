// slang-emit.cpp
#include "slang-emit.h"

#include "../core/slang-writer.h"
#include "slang-ir-bind-existentials.h"
#include "slang-ir-dce.h"
#include "slang-ir-entry-point-uniforms.h"
#include "slang-ir-glsl-legalize.h"
#include "slang-ir-insts.h"
#include "slang-ir-link.h"
#include "slang-ir-restructure.h"
#include "slang-ir-restructure-scoping.h"
#include "slang-ir-specialize.h"
#include "slang-ir-specialize-resources.h"
#include "slang-ir-ssa.h"
#include "slang-ir-union.h"
#include "slang-ir-validate.h"
#include "slang-legalize-types.h"
#include "slang-lower-to-ir.h"
#include "slang-mangle.h"
#include "slang-name.h"
#include "slang-syntax.h"
#include "slang-type-layout.h"
#include "slang-visitor.h"

#include "slang-emit-source-writer.h"

#include "slang-emit-c-like.h"

#include "slang-emit-glsl.h"
#include "slang-emit-hlsl.h"
#include "slang-emit-cpp.h"

#include <assert.h>

namespace Slang {

enum class BuiltInCOp
{
    Splat,                  //< Splat a single value to all values of a vector or matrix type
    Init,                   //< Initialize with parameters (must match the type)
};


//


//

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
        if(entryPointLayout->entryPoint.GetName() != entryPoint->getName())
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

static void dumpIR(
    BackEndCompileRequest* compileRequest,
    IRModule*       irModule,
    char const*     label)
{
    DiagnosticSinkWriter writerImpl(compileRequest->getSink());
    WriterHelper writer(&writerImpl);

    if(label)
    {
        writer.put("### ");
        writer.put(label);
        writer.put(":\n");
    }

    dumpIR(irModule, writer.getWriter());

    if( label )
    {
        writer.put("###\n");
    }
}

static void dumpIRIfEnabled(
    BackEndCompileRequest* compileRequest,
    IRModule*       irModule,
    char const*     label = nullptr)
{
    if(compileRequest->shouldDumpIR)
    {
        dumpIR(compileRequest, irModule, label);
    }
}

String emitEntryPoint(
    BackEndCompileRequest*  compileRequest,
    EntryPoint*             entryPoint,
    CodeGenTarget           target,
    TargetRequest*          targetRequest)
{
    auto sink = compileRequest->getSink();
    auto program = compileRequest->getProgram();
    auto targetProgram = program->getTargetProgram(targetRequest);
    auto programLayout = targetProgram->getOrCreateLayout(sink);

//    auto translationUnit = entryPoint->getTranslationUnit();

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
    desc.entryPoint = entryPoint;
    desc.effectiveProfile = getEffectiveProfile(entryPoint, targetRequest);
    desc.sourceWriter = &sourceWriter;

    if (entryPoint && programLayout)
    {
        desc.entryPointLayout = findEntryPointLayout(programLayout, entryPoint);
    }

    desc.programLayout = programLayout;

    // Layout information for the global scope is either an ordinary
    // `struct` in the common case, or a constant buffer in the case
    // where there were global-scope uniforms.
    
    StructTypeLayout* globalStructLayout = programLayout ? getGlobalStructLayout(programLayout) : nullptr;
    desc.globalStructLayout = globalStructLayout;

    RefPtr<CLikeSourceEmitter> sourceEmitter;

    typedef CLikeSourceEmitter::SourceStyle SourceStyle;

    SourceStyle sourceStyle = CLikeSourceEmitter::getSourceStyle(target);
    switch (sourceStyle)
    {
        case SourceStyle::CPP:
        {
            sourceEmitter = new CPPSourceEmitter(desc);
            break;
        }
        case SourceStyle::GLSL:
        {
            sourceEmitter = new GLSLSourceEmitter(desc);
            break;
        }
        case SourceStyle::HLSL:
        {
            sourceEmitter = new HLSLSourceEmitter(desc);
            break;
        }
        default: break;
    }

    if (!sourceEmitter)
    {
        sink->diagnose(SourceLoc(), Diagnostics::unableToGenerateCodeForTarget, getCodeGenTargetName(target));
        return String();
    }

    // Outside because we want to keep IR in scope whilst we are processing emits
    LinkedIR linkedIR;
    {
        auto session = targetRequest->getSession();

        // We start out by performing "linking" at the level of the IR.
        // This step will create a fresh IR module to be used for
        // code generation, and will copy in any IR definitions that
        // the desired entry point requires. Along the way it will
        // resolve references to imported/exported symbols across
        // modules, and also select between the definitions of
        // any "profile-overloaded" symbols.
        //
        linkedIR = linkIR(
            compileRequest,
            entryPoint,
            programLayout,
            target,
            targetRequest);
        auto irModule = linkedIR.module;
        auto irEntryPoint = linkedIR.entryPoint;

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
        //
        // An easy transformation of this kind is to take uniform
        // parameters of a shader entry point and move them into
        // the global scope instead.
        //
        moveEntryPointUniformParamsToGlobalScope(irModule, target);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "ENTRY POINT UNIFORMS MOVED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

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
        specializeModule(irModule);

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
        eliminateDeadCode(compileRequest, irModule);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER DCE");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // We don't need the legalize pass for C/C++ based types
        if (!(sourceStyle == SourceStyle::CPP || sourceStyle == SourceStyle::C))
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
            eliminateDeadCode(compileRequest, irModule);
        
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
            eliminateDeadCode(compileRequest, irModule);
        }

        //  Debugging output of legalization
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "LEGALIZED");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

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
        // function parameters is invalid. To clean this up,
        // we will try to specialize called functions based
        // on the actual resources that are being passed to them
        // at specific call sites.
        //
        // Because the legalization may depend on what target
        // we are compiling for (certain things might be okay
        // for D3D targets that are not okay for Vulkan), we
        // pass down the target request along with the IR.
        //
        specializeResourceParameters(compileRequest, targetRequest, irModule);

#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER RESOURCE SPECIALIZATION");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);


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
            legalizeEntryPointForGLSL(
                session,
                irModule,
                irEntryPoint,
                compileRequest->getSink(),
                sourceEmitter->getGLSLExtensionTracker());

#if 0
                dumpIRIfEnabled(compileRequest, irModule, "GLSL LEGALIZED");
#endif
                validateIRModuleIfEnabled(compileRequest, irModule);
        }
        break;

        default:
            break;
        }

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
        eliminateDeadCode(compileRequest, irModule);
#if 0
        dumpIRIfEnabled(compileRequest, irModule, "AFTER DCE");
#endif
        validateIRModuleIfEnabled(compileRequest, irModule);

        // After all of the required optimization and legalization
        // passes have been performed, we can emit target code from
        // the IR module.
        //
        // TODO: do we want to emit directly from IR, or translate the
        // IR back into AST for emission?
        sourceEmitter->emitModule(irModule);
    }

    // Deal with cases where a particular stage requires certain GLSL versions
    // and/or extensions.
    switch( entryPoint->getStage() )
    {
    default:
        break;

    case Stage::AnyHit:
    case Stage::Callable:
    case Stage::ClosestHit:
    case Stage::Intersection:
    case Stage::Miss:
    case Stage::RayGeneration:
        if( target == CodeGenTarget::GLSL )
        {
            sourceEmitter->getGLSLExtensionTracker()->requireExtension("GL_NV_ray_tracing");
            sourceEmitter->getGLSLExtensionTracker()->requireVersion(ProfileVersion::GLSL_460);
        }
        break;
    }

    String code = sourceWriter.getContent();
    sourceWriter.clearContent();

    // Now that we've emitted the code for all the declarations in the file,
    // it is time to stitch together the final output.

    // There may be global-scope modifiers that we should emit now
    sourceEmitter->emitPreprocessorDirectives();

    sourceEmitter->emitLayoutDirectives(targetRequest);

    String prefix = sourceWriter.getContent();
    
    StringBuilder finalResultBuilder;
    finalResultBuilder << prefix;

    finalResultBuilder << sourceEmitter->getGLSLExtensionTracker()->getExtensionRequireLines();

    finalResultBuilder << code;

    String finalResult = finalResultBuilder.ProduceString();

    return finalResult;
}

} // namespace Slang
