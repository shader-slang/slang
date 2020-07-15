// slang-ir-entry-point-uniforms.cpp
#include "slang-ir-entry-point-uniforms.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

#include "slang-mangle.h"

namespace Slang
{


// The transformations in this file will solve the problem of taking
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

// For clarity and flexibility, the work is split across two
// different IR passes:
//
// * The first pass simply collects together uniform parameters
//   into a single parameter of `struct` or `ConstantBuffer<...>` type.
//
// * The second pass transforms entry-point uniform parameters
//   into global shader parameters.

// First we start with some helper subroutines for detecting
// whether a parameter represents a varying input rather than
// a uniform parameter.


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

bool isVaryingParameter(IRTypeLayout* typeLayout)
{
    // If *any* of the resources consumed by the parameter type
    // is *not* a varying resource kind, then we consider the
    // whole parameter to be uniform (and thus not varying).
    //
    // Note that this means that an empty type will always
    // be considered varying, even if it had been explicitly
    // marked `uniform`.
    //
    // Note that this logic rules out support for parameters
    // that mix varying and non-varying resource kinds.
    //
    // TODO: This whole convoluted definition exists because
    // we currently don't give system-value parameters any
    // reosurce kind, so they show up as empty. Simply
    // adding `LayoutResourceKind`s for system-value inputs
    // and outputs would allow for simpler logic here.
    //
    for(auto sizeAttr : typeLayout->getSizeAttrs())
    {
        if(!isVaryingResourceKind(sizeAttr->getResourceKind()))
            return false;
    }
    return true;
}

bool isVaryingParameter(IRVarLayout* varLayout)
{
    return isVaryingParameter(varLayout->getTypeLayout());
}

// Our two passes have a fair amount in common in terms of
// how they traverse the IR, so we will factor out the
// shared logic into a base type.

struct PerEntryPointPass
{
    // We'll hang on to the module we are processing,
    // so that we can refer to it when setting up `IRBuilder`s.
    //
    IRModule* module;


    SharedIRBuilder* m_sharedBuilder = nullptr;

    // We will process a whole module by visiting all
    // its global functions, looking for entry points.
    //
    void processModule()
    {
        SharedIRBuilder sharedBuilder(module);
        m_sharedBuilder = &sharedBuilder;

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

            // If we find a candidate entry point, then we
            // will process it.
            //
            processEntryPoint(func);
        }
    }

    void processEntryPoint(IRFunc* entryPointFunc)
    {
        m_entryPointFunc = entryPointFunc;
        processEntryPointImpl(entryPointFunc);
    }

    IRFunc* m_entryPointFunc = nullptr;

    virtual void processEntryPointImpl(IRFunc* entryPointFunc) = 0;
};


struct CollectEntryPointUniformParams : PerEntryPointPass
{
    CollectEntryPointUniformParamsOptions m_options;

    // *If* the entry point has any uniform parameter then we want to create a
    // structure type to house them, and a single collected shader parameter (either
    // an instance of that type or a constant buffer).
    //
    // We only want to create these if actually needed, so we will declare
    // them here and then initialize them on-demand.
    //
    IRStructType* paramStructType = nullptr;
    IRParam* collectedParam = nullptr;

    IRVarLayout* entryPointParamsLayout = nullptr;
    bool needConstantBuffer = false;

    void processEntryPointImpl(IRFunc* entryPointFunc) SLANG_OVERRIDE
    {
        // This pass object may be used across multiple entry points,
        // so we need to make sure to reset state that could have been
        // left over from a previous entry point.
        //
        paramStructType = nullptr;
        collectedParam = nullptr;

        // We expect all entry points to have explicit layout information attached.
        //
        // We will assert that we have the information we need, but try to be
        // defensive and bail out in the failure case in release builds.
        //
        auto funcLayoutDecoration = entryPointFunc->findDecoration<IRLayoutDecoration>();
        SLANG_ASSERT(funcLayoutDecoration);
        if(!funcLayoutDecoration)
            return;

        auto entryPointLayout = as<IREntryPointLayout>(funcLayoutDecoration->getLayout());
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
        entryPointParamsLayout = entryPointLayout->getParamsLayout();
        needConstantBuffer = as<IRParameterGroupTypeLayout>(entryPointParamsLayout->getTypeLayout()) != nullptr; 

        auto entryPointParamsStructLayout = getScopeStructLayout(entryPointLayout);

        // We will set up an IR builder so that we are ready to generate code.
        //
        IRBuilder builderStorage(m_sharedBuilder);
        auto builder = &builderStorage;

        if(m_options.alwaysCreateCollectedParam)
            ensureCollectedParamAndTypeHaveBeenCreated();

        // We will be removing any uniform parameters we run into, so we
        // need to iterate the parameter list carefully to deal with
        // us modifying it along the way.
        //
        IRParam* nextParam = nullptr;
        UInt paramCounter = 0;
        for( IRParam* param = entryPointFunc->getFirstParam(); param; param = nextParam )
        {
            nextParam = param->getNextParam();
            UInt paramIndex = paramCounter++;

            // We expect all entry-point parameters to have layout information,
            // but we will be defensive and skip parameters without the required
            // information when we are in a release build.
            //
            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            SLANG_ASSERT(layoutDecoration);
            if(!layoutDecoration)
                continue;
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
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
            ensureCollectedParamAndTypeHaveBeenCreated();

            // Now that we've ensured the global `struct` type and collected shader paramter
            // exist, we need to add a field to the `struct` to represent the
            // current parameter.
            //

            auto paramType = param->getFullType();

            builder->setInsertBefore(paramStructType);

            // We need to know the "key" that should be used for the parameter,
            // so we will read it off of the entry-point layout information.
            //
            // TODO: Maybe we should associate the key to the parameter via
            // a decoration to avoid this indirection?
            //
            // TODO: Alternatively, we should make this pass responsible for
            // dealing with the transfer of layout information from the entry
            // point to its parameters, rather than baking that behavior into
            // the linker. After all, this pass is traversing the same information
            // anyway, so it could do the work while it is here...
            //
            auto paramFieldKey = cast<IRStructKey>(entryPointParamsStructLayout->getFieldLayoutAttrs()[paramIndex]->getFieldKey());

            auto paramField = builder->createStructField(paramStructType, paramFieldKey, paramType);
            SLANG_UNUSED(paramField);

            // We will transfer all decorations on the parameter over to the key
            // so that they can affect downstream emit logic.
            //
            // TODO: We should double-check whether any of the decorations should
            // be moved to the *field* instead.
            //
            param->transferDecorationsTo(paramFieldKey);

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
                        collectedParam,
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
                        collectedParam,
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

        if( collectedParam )
        {
            collectedParam->insertBefore(entryPointFunc->getFirstBlock()->getFirstChild());
        }

        fixUpFuncType(entryPointFunc);
    }

    void ensureCollectedParamAndTypeHaveBeenCreated()
    {
        if(paramStructType)
            return;

        IRBuilder builder(m_sharedBuilder);

        // First we create the structure to hold the parameters.
        //
        builder.setInsertBefore(m_entryPointFunc);
        paramStructType = builder.createStructType();
        builder.addNameHintDecoration(paramStructType, UnownedTerminatedStringSlice("EntryPointParams"));

        if( needConstantBuffer )
        {
            // If we need a constant buffer, then the global
            // shader parameter will be a `ConstantBuffer<paramStructType>`
            //
            auto constantBufferType = builder.getConstantBufferType(paramStructType);
            collectedParam = builder.createParam(constantBufferType);
        }
        else
        {
            // Otherwise, the global shader parameter is just
            // an instance of `paramStructType`.
            //
            collectedParam = builder.createParam(paramStructType);
        }

        // No matter what, the global shader parameter should have the layout
        // information from the entry point attached to it, so that the
        // contained parameters will end up in the right place(s).
        //
        builder.addLayoutDecoration(collectedParam, entryPointParamsLayout);

        // We add a name hint to the global parameter so that it will
        // emit to more readable code when referenced.
        //
        builder.addNameHintDecoration(collectedParam, UnownedTerminatedStringSlice("entryPointParams"));
    }
};

struct MoveEntryPointUniformParametersToGlobalScope : PerEntryPointPass
{
    void processEntryPointImpl(IRFunc* entryPointFunc) SLANG_OVERRIDE
    {
        // We will set up an IR builder so that we are ready to generate code.
        //
        IRBuilder builderStorage(m_sharedBuilder);
        auto builder = &builderStorage;

        builder->setInsertBefore(entryPointFunc);

        // We will be removing any uniform parameters we run into, so we
        // need to iterate the parameter list carefully to deal with
        // us modifying it along the way.
        //
        IRParam* nextParam = nullptr;
        for( IRParam* param = entryPointFunc->getFirstParam(); param; param = nextParam )
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
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
            SLANG_ASSERT(paramLayout);
            if(!paramLayout)
                continue;

            // A parameter that has varying input/output behavior should be left alone,
            // since this pass is only supposed to apply to uniform (non-varying)
            // parameters.
            //
            if(isVaryingParameter(paramLayout))
                continue;

            auto paramType = param->getFullType();

            builder->setInsertBefore(entryPointFunc);
            auto globalParam = builder->createGlobalParam(paramType);

            param->transferDecorationsTo(globalParam);

            // We also decorate the parameter for the entry-point parameters
            // so that we can find it again in downstream passes (like emit
            // for CPU/CUDA) that might want to treat entry-point parameters
            // different from other cases.
            //
            // TODO: Once we have support for multiple entry points to be emitted
            // at once, we need a way to associate these per-entry-point parameters
            // more closely with the original entry point. The two easiest options
            // are:
            //
            // 1. Don't move the new aggregate parameter to the global scope
            // on those targets, and instead keep it as a parameter of the
            // entry point.
            //
            // 2. Use a decoration on the entry point itself to point at the
            // global parameter for its per-entry-point parameter data.
            //
            builder->addDecoration(globalParam, kIROp_EntryPointParamDecoration);

            param->replaceUsesWith(globalParam);
            param->removeAndDeallocate();
        }

        fixUpFuncType(entryPointFunc);
    }
};

void collectEntryPointUniformParams(
    IRModule*                                       module,
    CollectEntryPointUniformParamsOptions const&    options)
{
    CollectEntryPointUniformParams context;
    context.module = module;
    context.m_options = options;
    context.processModule();
}

void moveEntryPointUniformParamsToGlobalScope(
    IRModule*   module)
{
    MoveEntryPointUniformParametersToGlobalScope context;
    context.module = module;
    context.processModule();
}

}
