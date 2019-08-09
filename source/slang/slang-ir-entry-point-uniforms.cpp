// slang-ir-entry-point-uniforms.cpp
#include "slang-ir-entry-point-uniforms.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "slang-mangle.h"

namespace Slang
{


// The transformation in this file will solve the problem of taking
// code like the following:
//
//      float4 fragmentMain(
//          uniform Texture2D    t,
//          uniform SamplerState s;
//          uniform float4       c,
//                  float2       uv : UV) : SV_Target
//      {
//          return t.Sample(s, uv) + c;
//      }
//
// and transforming into code like this:
//
//      struct Params
//      {
//          Texture2D    t;
//          SamplerState s;
//          float4       c;
//      }
//      ConstantBuffer<Params> params;
//
//      float4 fragmentMain(
//          float2 uv : UV) : SV_Target
//      {
//          return params.t.Sample(params.s, uv) + params.c;
//      }
//
// As can be seen in this example, the `uniform` parameters
// declared as entry point parameters have been moved into
// a `struct` declaration that we then use to declare a global
// shader parameter that is a `ConstantBuffer`. We then
// rewrite references to those parameters to refer to the
// contents of the new constant buffer instead.
//
// We perform this transformation after the target-specific
// linking step, because that will have attached layout information
// to the entry point and its parameters. We need that layout
// information so that we can:
//
// * Identify which parameters are uniform vs. varying.
// * Have an appropriate layout to attached to the synthesized
//   global shader parameter `params`.
//
// One additional wrinkle this pass has to deal with is that
// in the case where the shader doesn't have any "ordinary"
// uniform parameters like `c` (e.g., it only has resource/object
// parameters), we do *not* wrap the parameter `struct` in
// a `ConstantBuffer`. For example, suppose we have:
//
//      float4 fragmentMain(
//          uniform Texture2D    t,
//          uniform SamplerState s;
//                  float2       uv : UV) : SV_Target
//      {
//          return t.Sample(s, uv);
//      }
//
// In this case the output of the transformation should be:
//
//      struct Params
//      {
//          Texture2D    t;
//          SamplerState s;
//      }
//      Params params;
//
//      float4 fragmentMain(
//          float2 uv : UV) : SV_Target
//      {
//          return params.t.Sample(params.s, uv) + params.c;
//      }
//
// Note that this pass should always come before type legalization,
// which will take responsibility for turning a variable like
// `params` above into individual variables for the `t` and
// `s` fields.

// The overall structure here is similar to many other IR passes.
// We define a "context" structure to encapsulate the pass.
//
struct MoveEntryPointUniformParametersToGlobalScope
{
    // We'll hang on to the module we are processing,
    // so that we can refer to it when setting up `IRBuilder`s.
    //
    IRModule* module;

    // The target can determine how a variable is moved out into global scope
    CodeGenTarget codeGenTarget;

    // If true the target needs constant buffer wrapping (for uniforms say)
    bool targetNeedsConstantBuffer;

    // We will process a whole module by visiting all
    // its global functions, looking for entry points.
    //
    void processModule()
    {
        // Note that we are only looking at true global-scope
        // functions and not functions nested inside of
        // IR generics. When using generic entry points, this
        // pass should be run after the entry point(s) have
        // been specialized to their generic type parameters.

        for( auto inst : module->getGlobalInsts() )
        {
            // We are only interested in entry points.
            //
            // Every entry point must be a function.
            //
            auto func = as<IRFunc>(inst);
            if( !func )
                continue;

            // Entry points will always have the `[entryPoint]`
            // decoration to differentiate them from ordinary
            // functions.
            //
            // TODO: we could make `IREntryPoint` a subclass of
            // `IRFunc` if desired, to avoid having to attach
            // an explicit decoration to identify them.
            //
            if( !func->findDecorationImpl(kIROp_EntryPointDecoration) )
                continue;

            // If we fine a candidate entry point, then we
            // will process it.
            //
            processEntryPoint(func);
        }
    }

    void processEntryPoint(IRFunc* func)
    {
        // We expect all entry points to have explicit layout information attached.
        //
        // We will assert that we have the information we need, but try to be
        // defensive and bail out in the failure case in release builds.
        //
        auto funcLayoutDecoration = func->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(funcLayoutDecoration);
        if(!funcLayoutDecoration)
            return;

        auto entryPointLayout = as<EntryPointLayout>(funcLayoutDecoration->getLayout());
        SLANG_ASSERT(entryPointLayout);
        if(!entryPointLayout)
            return;

        // The parameter layout for an entry point will either be a structure
        // type layout, or a constant buffer (a case of parameter group)
        // wrapped around such a structure.
        //
        // If we are in the latter case we will need to make sure to allocate
        // an explicit IR constant buffer for that wrapper, 
        //
        auto entryPointParamsLayout = entryPointLayout->parametersLayout;
        bool needConstantBuffer = targetNeedsConstantBuffer && entryPointParamsLayout->typeLayout.is<ParameterGroupTypeLayout>(); 

        // We will set up an IR builder so that we are ready to generate code.
        //
        SharedIRBuilder sharedBuilderStorage;
        auto sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->module = module;
        sharedBuilder->session = module->getSession();

        IRBuilder builderStorage;
        auto builder = &builderStorage;
        builder->sharedBuilder = sharedBuilder;

        // *If* the entry point has any uniform parameter then we want to create a
        // structure type to house them, and a global shader parameter (either
        // an instance of that type or a constant buffer).
        //
        // We only want to create these if actually needed, so we will declare
        // them here and then initialize them on-demand.
        //
        IRStructType* paramStructType = nullptr;
        IRGlobalParam* globalParam = nullptr;

        // We will be removing any uniform parameters we run into, so we
        // need to iterate the parameter list carefully to deal with
        // us modifying it along the way.
        //
        IRParam* nextParam = nullptr;
        for( IRParam* param = func->getFirstParam(); param; param = nextParam )
        {
            nextParam = param->getNextParam();

            // We expect all entry-point parameters to have layout information,
            // but we will be defensive and skip parameters without the required
            // information when we are in a release build.
            //
            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            SLANG_ASSERT(layoutDecoration);
            if(!layoutDecoration)
                continue;
            auto paramLayout = as<VarLayout>(layoutDecoration->getLayout());
            SLANG_ASSERT(paramLayout);
            if(!paramLayout)
                continue;

            // A parameter that has varying input/output behavior should be left alone,
            // since this pass is only supposed to apply to uniform (non-varying)
            // parameters.
            //
            if(isVaryingParameter(paramLayout))
                continue;

            // At this point we know that `param` is not a varying shader parameter,
            // so that we want to turn it into an equivalent global shader parameter.
            //
            // If this is the first parameter we are running into, then we need
            // to deal with creating the structure type and global shader
            // parameter that our transformed entry point will use.
            //
            if( !paramStructType )
            {
                // First we create the structure to hold the parameters.
                //
                builder->setInsertBefore(func);
                paramStructType = builder->createStructType();

                if( needConstantBuffer )
                {
                    // If we need a constant buffer, then the global
                    // shader parameter will be a `ConstantBuffer<paramStructType>`
                    //
                    auto constantBufferType = builder->getConstantBufferType(paramStructType);
                    globalParam = builder->createGlobalParam(constantBufferType);
                }
                else
                {
                    // Otherwise, the global shader parameter is just
                    // an instance of `paramStructType`.
                    //
                    globalParam = builder->createGlobalParam(paramStructType);
                }

                // No matter what, the global shader parameter should have the layout
                // information from the entry point attached to it, so that the
                // contained parameters will end up in the right place(s).
                //
                builder->addLayoutDecoration(globalParam, entryPointParamsLayout);
            }

            // Now that we've ensured the global `struct` type and shader paramter
            // exist, we need to add a field to the `struct` to represent the
            // current parameter.
            //

            auto paramType = param->getFullType();

            builder->setInsertBefore(paramStructType);
            auto paramFieldKey = builder->createStructKey();
            auto paramField = builder->createStructField(paramStructType, paramFieldKey, paramType);
            SLANG_UNUSED(paramField);

            // We will transfer all decorations on the parameter over to the key
            // so that they can affect downstream emit logic.
            //
            // TODO: We should double-check whether any of the decorations should
            // be moved to the *field* instead.
            //
            param->transferDecorationsTo(paramFieldKey);

            // There is a bit of a hacky issue, where downstream passes (notably
            // type legalization) require the field keys for `struct` types to
            // have mangled names, because those mangled names will be used to
            // lookup field layout information inside of the layout information
            // for the `struct` type.
            //
            // TODO: We should fix that design choice in how layout information
            // is stored, to avoid the reliance on name strings.
            //
            builder->addExportDecoration(paramFieldKey, getMangledName(paramLayout->varDecl).getUnownedSlice());

            // At this point we want to eliminate the original entry point
            // parameter, in favor of the `struct` field we declared.
            // That required replacing any uses of the parameter with
            // appropriate code to pull out the field.
            //
            // We *could* extract the field at the start of the shader
            // and then do a `replaceAllUsesWith` to propragate it
            // down, but in practice we expect that it is better for
            // performance to "rematerialize" the value of a shader
            // parameter as close to where it is used as possible.
            //
            // We are therefore going to replace the uses one at a time.
            //
            while(auto use = param->firstUse )
            {
                // Given a `use` of the paramter, we will insert
                // the replacement code right before the instruction
                // that is doing the using.
                //
                builder->setInsertBefore(use->getUser());

                // The way to extract the field that corresponds
                // to the parameter depends on whether or not
                // we generated a constant buffer.
                //
                IRInst* fieldVal = nullptr;
                if( needConstantBuffer )
                {
                    // A constant buffer behaves like a pointer
                    // at the IR level, so we first do a pointer
                    // offset operation to compute what amounts
                    // to `&cb->field`, and then load from that address.
                    //
                    auto fieldAddress = builder->emitFieldAddress(
                        builder->getPtrType(paramType),
                        globalParam,
                        paramFieldKey);
                    fieldVal = builder->emitLoad(fieldAddress);
                }
                else
                {
                    // In the ordinary struct case, the parameter
                    // has an ordinary `struct` type (not a pointer),
                    // so we just extract the field directly.
                    //
                    fieldVal = builder->emitFieldExtract(
                        paramType,
                        globalParam,
                        paramFieldKey);
                }

                // We replace the value used at this use site, which
                // will have a side effect of making `use` no longer
                // be on the list of uses for `param`, so that when
                // we get back to the top of the loop the list of
                // uses will be shorter.
                //
                use->set(fieldVal);
            }

            // Once we've replaced all the uses of `param`, we
            // can go ahead and remove it completely.
            //
            param->removeAndDeallocate();
        }

        fixUpFuncType(func);
    }

    // We need to be able to determine if a parameter is logically
    // a "varying" parameter based on its layout.
    //
    bool isVaryingParameter(VarLayout* layout)
    {
        // If *any* of the resources consumed by the parameter
        // is a varying resource kind (e.g., varying input) then
        // we consider the whole parameter to be varying.
        //
        // This is reasonable because there is no way to declare
        // a parameter that mixes varying and non-varying fields.
        //
        for( auto resInfo : layout->resourceInfos )
        {
            if(isVaryingResourceKind(resInfo.kind))
                return true;
        }

        // TODO(JS): We probably want a more accurate way of determining if system semantic value
        // We can use the flags Flag::SemanticValue for one. But main issue with this test, is for some
        // targets currently (CPU) no resources are consumed. Perhaps this is fixed elsewhere by using a 'notional' resource.

        // Varying parameters with "system value" semantics currently show up as
        // consuming no resources, so we need to special-case that here.
        //
        // Note: an empty `struct` parameter would also show up the same way, but
        // we should eliminate any such parameters later on during type legalization.
        //
        if(layout->resourceInfos.getCount() == 0)
            return true;

        // if none of the above tests determined that the
        // parameter was varying, then we can safely consider
        // it to be non-varying (uniform):
        return false;
    }

    // In order to determine whether a parameter is varying based on its
    // layout, we need to know which resource kinds represent varying
    // shader parameters.
    //
    bool isVaryingResourceKind(LayoutResourceKind kind)
    {
        switch( kind )
        {
        default:
            return false;

            // Note: The set of cases that are considered
            // varying here would need to be extended if we
            // add more fine-grained resource kinds (e.g.,
            // if we ever add an explicit resource kind
            // for geometry shader output streams).
            //
            // Ordinary varying input/output:
        case LayoutResourceKind::VaryingInput:
        case LayoutResourceKind::VaryingOutput:
            //
            // Ray-tracing shader input/output:
        case LayoutResourceKind::CallablePayload:
        case LayoutResourceKind::HitAttributes:
        case LayoutResourceKind::RayPayload:
            return true;
        }
    }
};

void moveEntryPointUniformParamsToGlobalScope(
    IRModule*   module,
    CodeGenTarget target)
{
    MoveEntryPointUniformParametersToGlobalScope context;
    
    context.module = module;
    context.codeGenTarget = target;
    context.targetNeedsConstantBuffer = true;

    // Check if this target needs constant buffer wrapping
    switch (target)
    {
        case CodeGenTarget::CPPSource:
        case CodeGenTarget::CSource:
        case CodeGenTarget::Executable:
        case CodeGenTarget::SharedLibrary:
        case CodeGenTarget::HostCallable:
        {
            context.targetNeedsConstantBuffer = false;
            break;
        }
        default: break;
    }

    context.processModule();
}

}
