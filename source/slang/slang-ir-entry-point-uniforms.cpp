// slang-ir-entry-point-uniforms.cpp
#include "slang-ir-entry-point-uniforms.h"

#include "slang-ir-entry-point-pass.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
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

// Setup some flags that will be used below to check for varying
// resource kinds. Ordinary varying input/output + Ray-tracing shader
// input/output kinds will be considered varying.
//
// Note: The set of cases that are considered
// varying here would need to be extended if we
// add more fine-grained resource kinds (e.g.,
// if we ever add an explicit resource kind
// for geometry shader output streams).
static const LayoutResourceKindFlags flags =
    LayoutResourceKindFlag::make(LayoutResourceKind::VaryingInput) |
    LayoutResourceKindFlag::make(LayoutResourceKind::VaryingOutput) |
    LayoutResourceKindFlag::make(LayoutResourceKind::CallablePayload) |
    LayoutResourceKindFlag::make(LayoutResourceKind::HitAttributes) |
    LayoutResourceKindFlag::make(LayoutResourceKind::RayPayload);

bool isVaryingParameter(IRTypeLayout* typeLayout)
{
    if (!typeLayout)
        return false;

    // If we're looking at a struct type layout, we need to check each of the fields.
    // If any of the fields is not a varying resource kind, then we consider the
    // whole struct to be uniform (and thus not varying).
    if (auto structTypeLayout = as<IRStructTypeLayout>(typeLayout))
    {
        for (auto fieldAttr : structTypeLayout->getFieldLayoutAttrs())
        {
            if (!isVaryingParameter(fieldAttr->getLayout()))
                return false;
        }
        // If we made it here, all struct fields were varying.
        return true;
    }

    // If we're looking at a parameter group type layout, we should return false
    // as these should always be associtated with LayoutResourceKind::Uniform.
    else if (as<IRParameterGroupTypeLayout>(typeLayout))
    {
        return false;
    }

    // If we're looking at an array type layout, we need to check the element's
    // type layout.
    else if (auto arrayTypeLayout = as<IRArrayTypeLayout>(typeLayout))
    {
        return isVaryingParameter(arrayTypeLayout->getElementTypeLayout());
    }

    // If we didn't match a type layout above, try finding a kind from sizeAttrs.
    else
    {
        // If all the element type's sizeAttrs have a varying kind matching our
        // flags above, return true.
        for (auto sizeAttr : typeLayout->getSizeAttrs())
        {
            if ((flags & LayoutResourceKindFlag::make(sizeAttr->getResourceKind())) == 0)
                return false;
        }
        return true;
    }
}

bool isVaryingParameter(IRVarLayout* varLayout)
{
    // If *any* of the resources consumed by the parameter type
    // is *not* a varying resource kind, then we consider the
    // whole parameter to be uniform (and thus not varying).
    //
    // Note that this means that some empty types will always
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

    if (!varLayout)
        return false;

    // System-value parameters currently do not have kinds setup.
    // As a WAR, if we see a system-value attr just assume varying
    // for now.
    if (varLayout->findAttr<IRSystemValueSemanticAttr>())
    {
        return true;
    }

    if (varLayout->usesResourceFromKinds(flags))
    {
        return true;
    }

    // If the base cases above failed, we need to check if we are dealing with
    // an IRVarLayout that could have "nested" IRVarLayouts, such as an IRStructTypeLayout.
    return isVaryingParameter(varLayout->getTypeLayout());
}

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

    bool needConstantBuffer = false;

    void processEntryPointImpl(EntryPointInfo const& info) SLANG_OVERRIDE
    {
        auto entryPointFunc = info.func;

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

        // If the module contains two functions with entrypoint decorations,
        // and one entrypoint calls the other entrypoint, and the user
        // tells us to compile the caller entrypoint but not the callee
        // entrypoint, we will not have the layout decoration created for
        // the callee entrypoint. In this case, we should simply treat the
        // callee entrypoint as if it is an ordinary function and skip the
        // rest of the logic here.
        if (!funcLayoutDecoration)
            return;

        auto entryPointLayout = as<IREntryPointLayout>(funcLayoutDecoration->getLayout());
        SLANG_ASSERT(entryPointLayout);
        if (!entryPointLayout)
            return;

        // The parameter layout for an entry point will either be a structure
        // type layout, or a constant buffer (a case of parameter group)
        // wrapped around such a structure.
        //
        // If we are in the latter case we will need to make sure to allocate
        // an explicit IR constant buffer for that wrapper,
        //
        auto entryPointParamsLayout = entryPointLayout->getParamsLayout();
        needConstantBuffer =
            as<IRParameterGroupTypeLayout>(entryPointParamsLayout->getTypeLayout()) != nullptr;

        auto entryPointParamsStructLayout = getScopeStructLayout(entryPointLayout);

        // We will set up an IR builder so that we are ready to generate code.
        //
        IRBuilder builderStorage(m_module);
        auto builder = &builderStorage;

        if (m_options.alwaysCreateCollectedParam)
            ensureCollectedParamAndTypeHaveBeenCreated();

        IRStructTypeLayout::Builder structLayoutBuilder(builder);

        // We will be removing any uniform parameters we run into, so we
        // need to iterate the parameter list carefully to deal with
        // us modifying it along the way.
        //
        IRParam* nextParam = nullptr;
        UInt paramCounter = 0;
        HashSet<LayoutResourceKind> resourceKinds;
        for (IRParam* param = entryPointFunc->getFirstParam(); param; param = nextParam)
        {
            nextParam = param->getNextParam();
            UInt paramIndex = paramCounter++;

            // We expect all entry-point parameters to have layout information,
            // but we will be defensive and skip parameters without the required
            // information when we are in a release build.
            //
            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            SLANG_ASSERT(layoutDecoration);
            if (!layoutDecoration)
                continue;
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
            SLANG_ASSERT(paramLayout);
            if (!paramLayout)
                continue;

            // A parameter that has varying input/output behavior should be left alone,
            // since this pass is only supposed to apply to uniform (non-varying)
            // parameters.
            //
            if (isVaryingParameter(paramLayout))
                continue;

            for (auto offsetAttr : paramLayout->getOffsetAttrs())
                resourceKinds.add(offsetAttr->getResourceKind());

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
            auto paramFieldKey = cast<IRStructKey>(
                entryPointParamsStructLayout->getFieldLayoutAttrs()[paramIndex]->getFieldKey());
            structLayoutBuilder.addField(
                paramFieldKey,
                entryPointParamsStructLayout->getFieldLayout(paramIndex));

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
            while (auto use = param->firstUse)
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
                if (needConstantBuffer)
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
                    fieldVal = builder->emitFieldExtract(paramType, collectedParam, paramFieldKey);
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

        // Filter unrelated offset attrs from entryPointParamsLayout.
        // Since entryPointParamsLayout will now be used as the varLayout for the newly created
        // struct typed uniform parameter, and we need to ensure it only contains uniform parameter
        // offsets, not varying ones, so isVaryingParameter() can correctly identify the newly
        // created struct param as a uniform param.

        if (collectedParam)
        {
            collectedParam->insertBefore(entryPointFunc->getFirstBlock()->getFirstChild());
            IRTypeLayout* entryPointUniformsTypeLayout = nullptr;

            if (auto originalParamGroupLayout =
                    as<IRParameterGroupTypeLayout>(entryPointParamsLayout->getTypeLayout()))
            {
                // If the original entry point layout is a parameter gorup layout,
                // the new layout of the newly created `entryPointParams` param should also
                // be a parameter group layout, but with unrelated offsets (e.g. varying offsets)
                // stripped off.
                // We will now create this layout inst.
                //
                // Register any existing ResourceKinds on the container var layout as "related".
                for (auto offsetAttr :
                     originalParamGroupLayout->getContainerVarLayout()->getOffsetAttrs())
                {
                    resourceKinds.add(offsetAttr->getResourceKind());
                }
                // Create the struct layout for parameters that goes into the `EntryPointParams`
                // struct.
                auto entryPointUniformStructTypeLayout = structLayoutBuilder.build();
                auto originalElementVarLayout = originalParamGroupLayout->getElementVarLayout();
                IRVarLayout::Builder elementVarLayoutBuilder(
                    builder,
                    entryPointUniformStructTypeLayout);
                elementVarLayoutBuilder.cloneEverythingButOffsetsFrom(originalElementVarLayout);

                IRParameterGroupTypeLayout::Builder paramGroupTypeLayoutBuilder(builder);
                // Filter offsets for the `elementVarLayout` part of the new parameter group layout.
                for (auto resKind : resourceKinds)
                {
                    auto originalOffset = originalElementVarLayout->findOffsetAttr(resKind);
                    if (!originalOffset)
                        continue;
                    auto resInfo = elementVarLayoutBuilder.findOrAddResourceInfo(resKind);
                    resInfo->offset = originalOffset->getOffset();
                    resInfo->space = originalOffset->getSpace();
                }
                for (auto resKind : resourceKinds)
                {
                    if (auto sizeAttr = originalParamGroupLayout->findSizeAttr(resKind))
                        paramGroupTypeLayoutBuilder.addResourceUsage(sizeAttr);
                }
                auto newElementVarLayout = elementVarLayoutBuilder.build();
                // The "containerVarLayout" part should remain unchanged from the original layout.
                // Because this is where we store the offset of the default constant buffer itself.
                paramGroupTypeLayoutBuilder.setContainerVarLayout(
                    originalParamGroupLayout->getContainerVarLayout());
                // The "elementVarLayout" part should be the new one we just created.
                paramGroupTypeLayoutBuilder.setElementVarLayout(newElementVarLayout);
                // The "offsetElementTypeLayout" part is just redundant convenient info that
                // can be calculated from `entryPointUniformStructTypeLayout` and
                // `newElementVarLayout`.
                paramGroupTypeLayoutBuilder.setOffsetElementTypeLayout(applyOffsetToTypeLayout(
                    builder,
                    entryPointUniformStructTypeLayout,
                    newElementVarLayout));
                // Now we have the new type layout for the `EntryPointParams` parameter.
                entryPointUniformsTypeLayout = paramGroupTypeLayoutBuilder.build();
            }
            else
            {
                // If the original entry point layout isn't a constant buffer, we will simply use
                // the new struct type layout as the entrypoint layout.
                entryPointUniformsTypeLayout = structLayoutBuilder.build();
            }

            // Now create the var layout for the new `entryPointParams` parameter.
            // This can be done by simply filtering out unrelated offset attributes from the
            // original var layout.
            IRVarLayout::Builder varLayoutBuilder(builder, entryPointUniformsTypeLayout);
            varLayoutBuilder.cloneEverythingButOffsetsFrom(entryPointParamsLayout);
            List<IRVarOffsetAttr*> filteredOffsetAttrs;
            for (auto offsetAttr : entryPointParamsLayout->getOffsetAttrs())
            {
                if (resourceKinds.contains(offsetAttr->getResourceKind()))
                {
                    filteredOffsetAttrs.add(offsetAttr);
                }
            }
            for (auto offset : filteredOffsetAttrs)
            {
                auto resInfo = varLayoutBuilder.findOrAddResourceInfo(offset->getResourceKind());
                resInfo->offset = offset->getOffset();
                resInfo->space = offset->getSpace();
            }
            auto entryPointUniformsVarLayout = varLayoutBuilder.build();
            builder->addLayoutDecoration(collectedParam, entryPointUniformsVarLayout);
        }

        fixUpFuncType(entryPointFunc);
    }

    void ensureCollectedParamAndTypeHaveBeenCreated()
    {
        if (paramStructType)
            return;

        IRBuilder builder(m_module);

        // First we create the structure to hold the parameters.
        //
        builder.setInsertBefore(m_entryPoint.func);
        paramStructType = builder.createStructType();
        builder.addNameHintDecoration(
            paramStructType,
            UnownedTerminatedStringSlice("EntryPointParams"));
        builder.addBinaryInterfaceTypeDecoration(paramStructType);

        if (needConstantBuffer)
        {
            // If we need a constant buffer, then the global
            // shader parameter will be a `ConstantBuffer<paramStructType>`
            //
            IRType* layoutType = nullptr;

            if (m_options.targetReq->getOptionSet().getBoolOption(
                    CompilerOptionName::GLSLForceScalarLayout))
                layoutType = builder.getType(kIROp_ScalarBufferLayoutType);
            else if (m_options.targetReq->getOptionSet().getBoolOption(
                         CompilerOptionName::ForceCLayout))
                layoutType = builder.getType(kIROp_CBufferLayoutType);
            else if (isKhronosTarget(m_options.targetReq))
                layoutType = builder.getType(kIROp_Std430BufferLayoutType);
            else
                layoutType = builder.getType(kIROp_DefaultBufferLayoutType);
            auto constantBufferType = builder.getConstantBufferType(paramStructType, layoutType);
            collectedParam = builder.createParam(constantBufferType);
        }
        else
        {
            // Otherwise, the global shader parameter is just
            // an instance of `paramStructType`.
            //
            collectedParam = builder.createParam(paramStructType);
        }

        collectedParam->insertBefore(m_entryPoint.func);
        // We add a name hint to the global parameter so that it will
        // emit to more readable code when referenced.
        //
        builder.addNameHintDecoration(
            collectedParam,
            UnownedTerminatedStringSlice("entryPointParams"));
    }
};

struct MoveEntryPointUniformParametersToGlobalScope : PerEntryPointPass
{
    void processEntryPointImpl(EntryPointInfo const& info) SLANG_OVERRIDE
    {
        auto entryPointFunc = info.func;

        // We will set up an IR builder so that we are ready to generate code.
        //
        IRBuilder builderStorage(m_module);
        auto builder = &builderStorage;

        builder->setInsertBefore(entryPointFunc);

        // We will be removing any uniform parameters we run into, so we
        // need to iterate the parameter list carefully to deal with
        // us modifying it along the way.
        //
        IRParam* nextParam = nullptr;
        for (IRParam* param = entryPointFunc->getFirstParam(); param; param = nextParam)
        {
            nextParam = param->getNextParam();

            // We expect all entry-point parameters to have layout information,
            // but we will be defensive and skip parameters without the required
            // information when we are in a release build.
            //
            auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
            SLANG_ASSERT(layoutDecoration);
            if (!layoutDecoration)
                continue;
            auto paramLayout = as<IRVarLayout>(layoutDecoration->getLayout());
            SLANG_ASSERT(paramLayout);
            if (!paramLayout)
                continue;

            // A parameter that has varying input/output behavior should be left alone,
            // since this pass is only supposed to apply to uniform (non-varying)
            // parameters.
            //
            if (isVaryingParameter(paramLayout))
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
            // We need a way to associate these per-entry-point parameters
            // more closely with the original entry point. The two current
            // methods are:
            //
            // 1. Don't move the new aggregate parameter to the global scope
            // on those targets, and instead keep it as a parameter of the
            // entry point. This is used for CPU/CUDA targets.
            //
            // 2. Use a decoration on the global param itself to point at the
            // entry point for its per-entry-point parameter data, without moving
            // the parameter to the global scope. This is used for Metal targets, as
            // Metal does not have global parameters at the global scope.
            //
            // Method (1) is not used because Metal contains shading language concepts
            // such as binding offets, similar to other shading language targets.
            // We want to reuse code from other shading language targets for Metal, hence
            // we move parameters to the global scope, and then move the parameters back to
            // the entry points that they originate from. The originating entry points are
            // tracked through this decoration.
            //
            builder->addEntryPointParamDecoration(globalParam, entryPointFunc);

            param->replaceUsesWith(globalParam);
            param->removeAndDeallocate();
        }

        fixUpFuncType(entryPointFunc);
    }
};

void collectEntryPointUniformParams(
    IRModule* module,
    CollectEntryPointUniformParamsOptions const& options)
{
    CollectEntryPointUniformParams context;
    context.m_options = options;
    context.processModule(module);
}

void moveEntryPointUniformParamsToGlobalScope(IRModule* module)
{
    MoveEntryPointUniformParametersToGlobalScope context;
    context.processModule(module);
}

} // namespace Slang
