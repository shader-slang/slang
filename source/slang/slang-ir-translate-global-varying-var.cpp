#include "slang-ir-translate-global-varying-var.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
/// Return the first `VaryingInput` offset after the varying inputs that an entry point declares as
/// its own parameters, or 0 when it declares none. A module-scope input that is appended to the
/// entry point's parameters is placed relative to this offset: for `vsB(float4 extra : EXTRA)`,
/// `extra` takes location 0, so an appended `in float3 gPos;` starts at location 1.
static UInt getVaryingInputOffsetAfterParams(IRStructTypeLayout* paramsStructLayout)
{
    IRVarLayout* lastFieldVarLayout = nullptr;
    for (auto attr : paramsStructLayout->getFieldLayoutAttrs())
    {
        if (attr->getLayout()->usesResourceKind(LayoutResourceKind::VaryingInput))
            lastFieldVarLayout = attr->getLayout();
    }
    if (!lastFieldVarLayout)
        return 0;

    auto sizeAttr =
        lastFieldVarLayout->getTypeLayout()->findSizeAttr(LayoutResourceKind::VaryingInput);
    auto offsetAttr = lastFieldVarLayout->findOffsetAttr(LayoutResourceKind::VaryingInput);
    if (!sizeAttr || !offsetAttr)
        return 0;
    return sizeAttr->getFiniteSize() + (UInt)offsetAttr->getOffset();
}

/// Return true if the address `addr`, or an address derived from it by field or element access,
/// is passed to a function parameter that must be bound to a shader input variable. Consider:
///
///     in float4 vColor;
///     float4 rd() { return interpolateAtCentroid(vColor); }
///
/// `interpolateAtCentroid` carries `RequireShaderInputParameterDecoration(0)`, and its argument is
/// the address of `vColor`, so this returns true for `vColor`.
static bool isAddressPassedToShaderInputParameter(IRInst* addr)
{
    for (auto use = addr->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (auto call = as<IRCall>(user))
        {
            auto callee = getResolvedInstForDecorations(call->getCallee());
            auto requirement = callee->findDecoration<IRRequireShaderInputParameterDecoration>();
            if (requirement &&
                use == call->getArgs() + requirement->getParameterIndex()->getValue())
                return true;
        }
        else if (as<IRFieldAddress>(user) || as<IRGetElementPtr>(user))
        {
            if (isAddressPassedToShaderInputParameter(user))
                return true;
        }
    }
    return false;
}

struct GlobalVarTranslationContext
{
    CodeGenContext* context;

    /// The stage and `VaryingInput` location at which an entry point would read a module-scope
    /// input. `location` is unused for an input with a system-value semantic, which has no
    /// location.
    struct InterpolantInputPlacement
    {
        Stage stage;
        bool hasSystemValueSemantic;
        UInt location;
    };

    /// The entry point that receives a module-scope input passed to an interpolation function,
    /// and where it places that input.
    struct InterpolantInputOwner
    {
        IRFunc* entryPoint;
        InterpolantInputPlacement placement;
    };

    /// Return where `entryPoint` would place the module-scope input `input`, following the same
    /// rules that the translation loop in `processModule` uses to build the input's layout.
    InterpolantInputPlacement getInterpolantInputPlacement(IRInst* input, IRFunc* entryPoint)
    {
        InterpolantInputPlacement placement;
        placement.stage =
            entryPoint->findDecoration<IREntryPointDecoration>()->getProfile().getStage();
        placement.hasSystemValueSemantic = input->findDecoration<IRSemanticDecoration>() != nullptr;
        placement.location = 0;
        if (placement.hasSystemValueSemantic)
            return placement;

        if (auto layoutDecor = entryPoint->findDecoration<IRLayoutDecoration>())
        {
            if (auto entryPointLayout = as<IREntryPointLayout>(layoutDecor->getLayout()))
                placement.location =
                    getVaryingInputOffsetAfterParams(getScopeStructLayout(entryPointLayout));
        }
        if (auto varLayout = findVarLayout(input))
        {
            if (auto offsetAttr = varLayout->findOffsetAttr(LayoutResourceKind::VaryingInput))
                placement.location += (UInt)offsetAttr->getOffset();
        }
        return placement;
    }

    /// Diagnose `entryPoint` reaching the module-scope input `input` that `owner.entryPoint`
    /// already receives, if the two entry points would place it differently. `entryPoint` gets
    /// no parameter for `input`, so it reads the owner's input variable.
    void checkInterpolantInputPlacement(
        IRInst* input,
        InterpolantInputOwner const& owner,
        IRFunc* entryPoint)
    {
        auto placement = getInterpolantInputPlacement(input, entryPoint);
        bool samePlacement =
            placement.stage == owner.placement.stage &&
            (placement.hasSystemValueSemantic || placement.location == owner.placement.location);
        if (samePlacement)
            return;
        context->getSink()->diagnose(Diagnostics::SharedGlobalInterpolantInputMismatch{
            .inst = input,
            .entryPoint = entryPoint,
            .firstEntryPoint = owner.entryPoint,
            .location = input->sourceLoc});
    }

    void processModule(IRModule* module)
    {
        Dictionary<IRInst*, HashSet<IRFunc*>> referencingEntryPoints;
        buildEntryPointReferenceGraph(referencingEntryPoints, module);

        List<IRInst*> entryPoints;
        List<IRInst*> getWorkGroupSizeInsts;

        // Traverse the module to find all entry points.
        // If we see a `GetWorkGroupSize` instruction, we will materialize it.
        //
        for (auto inst : module->getGlobalInsts())
        {
            if (inst->getOp() == kIROp_Func && inst->findDecoration<IREntryPointDecoration>())
                entryPoints.add(inst);
            else if (inst->getOp() == kIROp_GetWorkGroupSize)
                getWorkGroupSizeInsts.add(inst);
        }
        for (auto inst : getWorkGroupSizeInsts)
            materializeGetWorkGroupSize(module, referencingEntryPoints, inst);
        IRBuilder builder(module);

        // A global varying may be reached from several entry points that are emitted into one
        // module. Each of those entry points receives the varying through its own parameter or
        // result field, so we copy the global's decorations onto every entry point's struct key
        // and only remove them from the global once all entry points have been processed.
        OrderedHashSet<IRInst*> translatedGlobalVars;

        // An interpolation function such as `interpolateAtCentroid` reads its argument through
        // the address of a real shader input variable, so a module-scope input whose address is
        // passed to one cannot become a private copy that each entry point initializes. Such an
        // input belongs to the first entry point that reaches it: that entry point receives it
        // as a parameter, and every other entry point reads the same input variable through the
        // shadowing relationship established below. Sharing one input variable is only valid if
        // every entry point would assign it the same stage and location, which we check here.
        Dictionary<IRInst*, InterpolantInputOwner> interpolantInputOwners;

        for (auto entryPoint : entryPoints)
        {
            auto entryPointFunc = as<IRFunc>(entryPoint);
            List<IRInst*> outputVars;
            List<IRInst*> inputVars;
            for (auto inst : module->getGlobalInsts())
            {
                if (auto referencingEntryPointSet = referencingEntryPoints.tryGetValue(inst))
                {
                    if (referencingEntryPointSet->contains(entryPointFunc))
                    {
                        if (inst->findDecoration<IRGlobalOutputDecoration>())
                        {
                            outputVars.add(inst);
                        }
                        if (inst->findDecoration<IRGlobalInputDecoration>())
                        {
                            if (auto owner = interpolantInputOwners.tryGetValue(inst))
                                checkInterpolantInputPlacement(inst, *owner, entryPointFunc);
                            else
                                inputVars.add(inst);
                        }
                    }
                }
            }

            bool hasInput = inputVars.getCount() != 0;
            bool hasOutput = outputVars.getCount() != 0;

            if (!hasInput && !hasOutput)
                continue;

            if (!entryPointFunc)
                continue;

            auto entryPointDecor = entryPointFunc->findDecoration<IREntryPointDecoration>();

            IRVarLayout* resultVarLayout = nullptr;
            IRVarLayout* paramLayout = nullptr;
            IRType* resultType = entryPointFunc->getResultType();

            // Create a struct type to receive all inputs.
            builder.setInsertBefore(entryPointFunc);
            auto inputStructType = builder.createStructType();
            IRStructTypeLayout::Builder inputStructTypeLayoutBuilder(&builder);

            // Go through the existing parameter layouts and add them to the
            // replacement struct type first.
            //
            // We'll also need to fix up the offsets of the global varing inputs
            // being added if we have any varing input entry point params.
            UInt nextOffset = 0;

            // If we have an existing entry point struct layout, we need to go through it and
            // add any of the struct field layout attributes as fields in the new struct type
            // layout that we are building to combine global varing ins and entry point ins.
            IRLayoutDecoration* entryPointLayoutDecor =
                entryPointFunc->findDecoration<IRLayoutDecoration>();
            IREntryPointLayout* entryPointLayout = nullptr;
            IRStructTypeLayout* entryPointParamsStructLayout = nullptr;

            if (entryPointLayoutDecor)
            {
                entryPointLayout = as<IREntryPointLayout>(entryPointLayoutDecor->getLayout());
                if (entryPointLayout)
                {
                    // The parameter layout for an entry point will either be a structure
                    // type layout, or a parameter group type layout.
                    entryPointParamsStructLayout = getScopeStructLayout(entryPointLayout);
                    if (entryPointParamsStructLayout)
                    {
                        for (auto attr : entryPointParamsStructLayout->getFieldLayoutAttrs())
                        {
                            // Add existing parameter layouts to the replacement struct type layout.
                            IRVarLayout* fieldLayout = attr->getLayout();
                            IRInst* key = attr->getFieldKey();
                            inputStructTypeLayoutBuilder.addField(key, fieldLayout);
                        }
                        nextOffset = getVaryingInputOffsetAfterParams(entryPointParamsStructLayout);
                    }
                }
            }

            // Add the global vars to the replacement struct and add new params to
            // the entry point func based on the global input vars.
            List<IRInst*> newParams;
            UInt inputVarIndex = 0;
            List<IRStructKey*> inputKeys;

            // Setup the location where we will insert the new params
            auto firstBlock = entryPointFunc->getFirstBlock();
            builder.setInsertInto(firstBlock);

            for (auto input : inputVars)
            {
                auto inputType = cast<IRPtrTypeBase>(input->getDataType())->getValueType();
                auto key = builder.createStructKey();
                inputKeys.add(key);
                builder.createStructField(inputStructType, key, inputType);

                IRTypeLayout::Builder fieldTypeLayoutBuilder(&builder);
                IRTypeLayout* fieldTypeLayout = nullptr;
                bool hasExistingLayout = false;
                if (auto existingLayoutDecoration = input->findDecoration<IRLayoutDecoration>())
                {
                    if (auto existingVarLayout =
                            as<IRVarLayout>(existingLayoutDecoration->getLayout()))
                    {
                        fieldTypeLayout = existingVarLayout->getTypeLayout();
                        hasExistingLayout = true;
                    }
                }

                if (!hasExistingLayout)
                {
                    fieldTypeLayout = fieldTypeLayoutBuilder.build();
                }

                IRVarLayout::Builder varLayoutBuilder(&builder, fieldTypeLayout);
                varLayoutBuilder.setStage(entryPointDecor->getProfile().getStage());
                if (auto semanticDecor = input->findDecoration<IRSemanticDecoration>())
                {
                    varLayoutBuilder.setSystemValueSemantic(
                        semanticDecor->getSemanticName(),
                        semanticDecor->getEffectiveSemanticIndex());
                }
                else
                {
                    // Start off the offset as nextOffset. If the global "in"s have existing
                    // offsetAttr's, we will add the offsets from those as well.
                    auto resInfo =
                        varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::VaryingInput);
                    resInfo->offset = nextOffset;

                    if (auto layoutDecor = findVarLayout(input))
                    {
                        if (auto offsetAttr =
                                layoutDecor->findOffsetAttr(LayoutResourceKind::VaryingInput))
                        {
                            resInfo->offset += (UInt)offsetAttr->getOffset();
                        }
                    }
                    if (entryPointDecor->getProfile().getStage() == Stage::Fragment)
                    {
                        varLayoutBuilder.setUserSemantic("COLOR", inputVarIndex);
                        if (!key->findDecoration<IRSemanticDecoration>())
                            builder.addSemanticDecoration(key, toSlice("COLOR"), inputVarIndex);
                    }
                    else if (entryPointDecor->getProfile().getStage() == Stage::Vertex)
                    {
                        varLayoutBuilder.setUserSemantic("VERTEX_IN_", inputVarIndex);
                        if (!key->findDecoration<IRSemanticDecoration>())
                            builder.addSemanticDecoration(
                                key,
                                toSlice("VERTEX_IN_"),
                                inputVarIndex);
                    }
                    inputVarIndex++;
                }
                auto varLayout = varLayoutBuilder.build();
                inputStructTypeLayoutBuilder.addField(key, varLayout);
                for (auto decoration : input->getDecorations())
                    cloneDecoration(decoration, key);
                translatedGlobalVars.add(input);

                // Emit a new param here to represent the global input var.
                auto inputParam =
                    builder.emitParam(builder.getBorrowInParamType(inputType, AddressSpace::Input));

                // Copy the global input vars original decorations onto the new param.
                // We need to do this to ensure that we can do things like get system
                // value info in later passes like when we legalize entry points for WGSL.
                IRCloneEnv cloneEnv;
                cloneInstDecorationsAndChildren(&cloneEnv, builder.getModule(), key, inputParam);

                // Add the layout to the new param
                builder.addLayoutDecoration(inputParam, varLayout);

                // Add the param to our list of new params. This will allow us
                // to connect the old "global variable" to a "global parameter"
                // later on.
                newParams.add(inputParam);
            }

            // Build the new layout that will replace the old entryPointLayout.
            auto paramTypeLayout = inputStructTypeLayoutBuilder.build();
            IRVarLayout::Builder paramVarLayoutBuilder(&builder, paramTypeLayout);
            paramLayout = paramVarLayoutBuilder.build();

            // Initialize all global variables in the order of struct member declaration.
            for (Index i = inputVars.getCount() - 1; i >= 0; i--)
            {
                auto input = inputVars[i];
                setInsertBeforeOrdinaryInst(&builder, firstBlock->getFirstOrdinaryInst());
                // TODO: Does the below TODO apply if we no longer use emitFieldExtract?
#if 0
                // TODO: This could be more efficient as a Load(FieldAddress(inputParam, i))
                // operation instead of a FieldExtract(Load(inputParam)).
                builder.emitStore(
                    input,
                    builder
                        .emitFieldExtract(inputType, builder.emitLoad(inputParam), inputKeys[i]));
#endif
                builder.emitStore(input, builder.emitLoad(newParams[i]));

                // Relate the old "global variable" to a "global parameter" for use later in
                // compilation to resolve a "global variable" shadowing a "global parameter"
                // relationship. GLSL/SPIR-V legalization then rewrites every use of `input` in
                // the module to this entry point's parameter, which is only valid when no other
                // entry point reaches `input`. A global shared by several entry points stays a
                // private variable that each entry point initializes with the store above.
                bool isInterpolantInput = isAddressPassedToShaderInputParameter(input);
                if (isInterpolantInput)
                {
                    interpolantInputOwners.add(
                        input,
                        InterpolantInputOwner{
                            entryPointFunc,
                            getInterpolantInputPlacement(input, entryPointFunc)});
                }
                if (isInterpolantInput || referencingEntryPoints.getValue(input).getCount() == 1)
                    builder.addGlobalVariableShadowingGlobalParameterDecoration(
                        newParams[i],
                        input,
                        inputKeys[i]);
            }

            // For each entry point, introduce a new parameter to represent each output parameter,
            // and return all outputs via a struct value.
            if (hasOutput)
            {
                // If we have global outputs, the entry-point must not return anything itself.
                if (as<IRFuncType>(entryPoint->getDataType())->getResultType()->getOp() !=
                    kIROp_VoidType)
                {
                    context->getSink()->diagnose(
                        Diagnostics::EntryPointMustReturnVoidWhenGlobalOutputPresent{
                            .location = entryPointFunc->sourceLoc});
                    continue;
                }
                builder.setInsertBefore(entryPointFunc);
                resultType = builder.createStructType();
                IRStructTypeLayout::Builder typeLayoutBuilder(&builder);
                UInt outputVarIndex = 0;
                for (auto output : outputVars)
                {
                    // The declared semantic and layout of an `in out` global describe its input,
                    // so its output field takes the stage's default semantic instead.
                    IRInst* outputDecorSource =
                        output->findDecoration<IRGlobalInputDecoration>() ? nullptr : output;
                    auto key = builder.createStructKey();
                    auto ptrType = as<IRPtrTypeBase>(output->getDataType());
                    builder.createStructField(resultType, key, ptrType->getValueType());

                    IRTypeLayout::Builder fieldTypeLayoutBuilder(&builder);
                    IRTypeLayout* fieldTypeLayout = nullptr;
                    bool hasExistingLayout = false;
                    if (auto existingLayoutDecoration =
                            outputDecorSource
                                ? outputDecorSource->findDecoration<IRLayoutDecoration>()
                                : nullptr)
                    {
                        if (auto existingVarLayout =
                                as<IRVarLayout>(existingLayoutDecoration->getLayout()))
                        {
                            fieldTypeLayout = existingVarLayout->getTypeLayout();
                            hasExistingLayout = true;
                        }
                    }

                    if (!hasExistingLayout)
                    {
                        fieldTypeLayout = fieldTypeLayoutBuilder.build();
                    }

                    IRVarLayout::Builder varLayoutBuilder(&builder, fieldTypeLayout);
                    varLayoutBuilder.setStage(entryPointDecor->getProfile().getStage());
                    if (auto semanticDecor =
                            outputDecorSource
                                ? outputDecorSource->findDecoration<IRSemanticDecoration>()
                                : nullptr)
                    {
                        varLayoutBuilder.setSystemValueSemantic(
                            semanticDecor->getSemanticName(),
                            semanticDecor->getEffectiveSemanticIndex());
                    }
                    else
                    {
                        if (auto layoutDecor =
                                outputDecorSource ? findVarLayout(outputDecorSource) : nullptr)
                        {
                            if (auto offsetAttr =
                                    layoutDecor->findOffsetAttr(LayoutResourceKind::VaryingOutput))
                            {
                                varLayoutBuilder
                                    .findOrAddResourceInfo(LayoutResourceKind::VaryingOutput)
                                    ->offset = (UInt)offsetAttr->getOffset();
                            }
                        }
                        if (entryPointDecor->getProfile().getStage() == Stage::Fragment)
                        {
                            varLayoutBuilder.setSystemValueSemantic("SV_TARGET", outputVarIndex);
                            if (!key->findDecoration<IRSemanticDecoration>())
                                builder.addSemanticDecoration(
                                    key,
                                    toSlice("SV_TARGET"),
                                    outputVarIndex);
                        }
                        else if (entryPointDecor->getProfile().getStage() == Stage::Vertex)
                        {
                            varLayoutBuilder.setUserSemantic("COLOR", outputVarIndex);
                            if (!key->findDecoration<IRSemanticDecoration>())
                                builder.addSemanticDecoration(
                                    key,
                                    toSlice("COLOR"),
                                    outputVarIndex);
                        }
                        outputVarIndex++;
                    }
                    typeLayoutBuilder.addField(key, varLayoutBuilder.build());
                    if (outputDecorSource)
                    {
                        for (auto decoration : outputDecorSource->getDecorations())
                            cloneDecoration(decoration, key);
                    }
                    translatedGlobalVars.add(output);
                }
                auto resultTypeLayout = typeLayoutBuilder.build();
                IRVarLayout::Builder resultVarLayoutBuilder(&builder, resultTypeLayout);
                resultVarLayout = resultVarLayoutBuilder.build();

                for (auto block : entryPointFunc->getBlocks())
                {
                    if (auto returnInst = as<IRReturn>(block->getTerminator()))
                    {
                        // Return the struct value.
                        builder.setInsertBefore(returnInst);
                        List<IRInst*> fieldVals;
                        for (auto outputVar : outputVars)
                        {
                            auto load = builder.emitLoad(outputVar);
                            fieldVals.add(load);
                        }
                        auto resultVal = builder.emitMakeStruct(
                            resultType,
                            (UInt)fieldVals.getCount(),
                            fieldVals.getBuffer());
                        builder.emitReturn(resultVal);
                        returnInst->removeAndDeallocate();
                    }
                }
            }
            if (entryPointLayout)
            {
                // Entry points with identical original layouts share one hoisted
                // `EntryPointLayout`, so we build a new layout for this entry point instead of
                // modifying the shared one in place.
                IRVarLayout* newParamsLayout = entryPointLayout->getParamsLayout();
                if (paramLayout)
                {
                    // We try to respect the original entry point type layout when replacing it
                    // below. The IRParameterGroupTypeLayout type layout is used when there are
                    // things like uniform entry point params, otherwise the IRStructTypeLayout type
                    // layout will normally represent the entry point type layout.
                    IRTypeLayout* newParamsTypeLayout = nullptr;
                    if (auto paramGroupTypeLayout =
                            as<IRParameterGroupTypeLayout>(entryPointParamsStructLayout))
                    {
                        // Build a new IRParameterGroupTypeLayout to replace the old one
                        // representing the entryPointLayout. The ContainerVarLayout shouldn't have
                        // changed, so resue that. Replace the rest with the new paramTypeLayout and
                        // paramLayout that we calculated above
                        IRParameterGroupTypeLayout::Builder paramGroupTypeLayoutBuilder(&builder);
                        paramGroupTypeLayoutBuilder.setContainerVarLayout(
                            paramGroupTypeLayout->getContainerVarLayout());
                        paramGroupTypeLayoutBuilder.setElementVarLayout(paramLayout);
                        paramGroupTypeLayoutBuilder.setOffsetElementTypeLayout(paramTypeLayout);
                        newParamsTypeLayout = paramGroupTypeLayoutBuilder.build();
                    }
                    else if (as<IRStructTypeLayout>(entryPointParamsStructLayout))
                    {
                        newParamsTypeLayout = paramTypeLayout;
                    }
                    else
                    {
                        SLANG_UNEXPECTED("uhandled global-scope binding layout");
                    }
                    List<IRInst*> paramsLayoutOperands;
                    paramsLayoutOperands.add(newParamsTypeLayout);
                    for (UInt i = 1; i < newParamsLayout->getOperandCount(); i++)
                        paramsLayoutOperands.add(newParamsLayout->getOperand(i));
                    newParamsLayout = builder.getVarLayout(paramsLayoutOperands);
                }
                IRVarLayout* newResultLayout =
                    resultVarLayout ? resultVarLayout : entryPointLayout->getResultLayout();
                entryPointLayoutDecor->removeAndDeallocate();
                builder.addLayoutDecoration(
                    entryPointFunc,
                    builder.getEntryPointLayout(newParamsLayout, newResultLayout));
            }
            // Update func type for the entry point.
            List<IRType*> paramTypes;
            for (auto param : entryPointFunc->getParams())
            {
                paramTypes.add(param->getDataType());
            }
            IRType* newFuncType = builder.getFuncType(paramTypes, resultType);
            entryPointFunc->setFullType(newFuncType);
        }

        for (auto globalVar : translatedGlobalVars)
        {
            while (auto decoration = globalVar->getFirstDecoration())
                decoration->removeAndDeallocate();
        }
    }

    // If we see a `GetWorkGroupSize` instruction, we should materialize it by replacing its uses
    // with a constant that represent the workgroup size of the calling entrypoint. This is trivial
    // if the `GetWorkGroupSize` instruction is used from a function called by one entry point. If
    // it is used in a place reachable from multiple entry points, we will introduce a global
    // variable to represent the workgroup size, and replace the uses with a load from the global
    // variable. We will assign the value of the global variable at the start of each entry point.
    //
    void materializeGetWorkGroupSize(
        IRModule* module,
        Dictionary<IRInst*, HashSet<IRFunc*>>& referenceGraph,
        IRInst* workgroupSizeInst)
    {
        IRBuilder builder(workgroupSizeInst);
        traverseUses(
            workgroupSizeInst,
            [&](IRUse* use)
            {
                if (auto parentFunc = getParentFunc(use->getUser()))
                {
                    auto referenceSet = referenceGraph.tryGetValue(parentFunc);
                    if (!referenceSet)
                        return;
                    if (referenceSet->getCount() == 1)
                    {
                        // If the function that uses the workgroup size is only used by one entry
                        // point, we can materialize the workgroup size by substituting the use with
                        // a constant.
                        auto entryPoint = *referenceSet->begin();
                        auto numthreadsDecor = entryPoint->findDecoration<IRNumThreadsDecoration>();
                        if (!numthreadsDecor)
                            return;
                        builder.setInsertBefore(use->getUser());
                        IRInst* values[3] = {
                            numthreadsDecor->getOperand(0),
                            numthreadsDecor->getOperand(1),
                            numthreadsDecor->getOperand(2)};

                        auto workgroupSize = builder.emitMakeVector(
                            builder.getVectorType(builder.getIntType(), 3),
                            3,
                            values);
                        builder.replaceOperand(use, workgroupSize);
                    }
                }
            });

        // If workgroupSizeInst still has uses, it means it is used by multiple entry points.
        // We need to introduce a global variable and assign value to it in each entry point.

        if (!workgroupSizeInst->hasUses())
        {
            workgroupSizeInst->removeAndDeallocate();
            return;
        }
        builder.setInsertBefore(workgroupSizeInst);
        auto globalVar = builder.createGlobalVar(workgroupSizeInst->getFullType());

        // Replace all remaining uses of the workgroupSize inst of a load from globalVar.
        traverseUses(
            workgroupSizeInst,
            [&](IRUse* use)
            {
                builder.setInsertBefore(use->getUser());
                auto load = builder.emitLoad(globalVar);
                builder.replaceOperand(use, load);
            });

        // Now insert assignments from each entry point.
        for (auto globalInst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(getResolvedInstForDecorations(globalInst));
            if (!func)
                continue;
            if (auto numthreadsDecor = func->findDecoration<IRNumThreadsDecoration>())
            {
                auto firstBlock = func->getFirstBlock();
                if (!firstBlock)
                    continue;
                builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
                IRInst* args[3] = {
                    numthreadsDecor->getOperand(0),
                    numthreadsDecor->getOperand(1),
                    numthreadsDecor->getOperand(2)};
                auto workgroupSize =
                    builder.emitMakeVector(workgroupSizeInst->getFullType(), 3, args);
                builder.emitStore(globalVar, workgroupSize);
            }
        }

        workgroupSizeInst->removeAndDeallocate();
    }
};

void translateGlobalVaryingVar(IRModule* module, CodeGenContext* context)
{
    GlobalVarTranslationContext ctx;
    ctx.context = context;
    ctx.processModule(module);
}
} // namespace Slang
