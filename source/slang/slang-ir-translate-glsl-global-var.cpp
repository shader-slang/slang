#include "slang-ir-translate-glsl-global-var.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-call-graph.h"

namespace Slang
{
    struct GlobalVarTranslationContext
    {
        CodeGenContext* context;

        void processModule(IRModule* module)
        {
            Dictionary<IRInst*, HashSet<IRFunc*>> referencingEntryPoints;
            buildEntryPointReferenceGraph(referencingEntryPoints, module);

            List<IRInst*> entryPoints;
            // Traverse the module to find all entry points.
            // If we see a `GetWorkGroupSize` instruction, we will materialize it.
            // 
            for (auto inst : module->getGlobalInsts())
            {
                if (inst->getOp() == kIROp_Func && inst->findDecoration<IREntryPointDecoration>())
                    entryPoints.add(inst);
                else if (inst->getOp() == kIROp_GetWorkGroupSize)
                    materializeGetWorkGroupSize(module, referencingEntryPoints, inst);
            }

            IRBuilder builder(module);

            for (auto entryPoint : entryPoints)
            {
                List<IRInst*> outputVars;
                List<IRInst*> inputVars;
                for (auto inst : module->getGlobalInsts())
                {
                    if (auto referencingEntryPointSet = referencingEntryPoints.tryGetValue(inst))
                    {
                        if (referencingEntryPointSet->contains((IRFunc*)entryPoint))
                        {
                            if (inst->findDecoration<IRGlobalOutputDecoration>())
                            {
                                outputVars.add(inst);
                            }
                            if (inst->findDecoration<IRGlobalInputDecoration>())
                            {
                                inputVars.add(inst);
                            }
                        }
                    }
                }

                bool hasInput = inputVars.getCount() != 0;
                bool hasOutput = outputVars.getCount() != 0;

                if (!hasInput && !hasOutput)
                    continue;

                auto entryPointFunc = as<IRFunc>(entryPoint);
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
                UInt inputVarIndex = 0;
                List<IRStructKey*> inputKeys;
                for (auto input : inputVars)
                {
                    auto inputType = cast<IRPtrTypeBase>(input->getDataType())->getValueType();
                    auto key = builder.createStructKey();
                    inputKeys.add(key);
                    if (auto nameHint = input->findDecoration<IRNameHintDecoration>())
                    {
                        builder.addNameHintDecoration(key, nameHint->getName());
                    }
                    builder.createStructField(inputStructType, key, inputType);
                    IRTypeLayout::Builder fieldTypeLayout(&builder);
                    IRVarLayout::Builder varLayoutBuilder(&builder, fieldTypeLayout.build());
                    varLayoutBuilder.setStage(entryPointDecor->getProfile().getStage());
                    if (auto locationDecoration = input->findDecoration<IRGLSLLocationDecoration>())
                    {
                        varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::VaryingInput)->offset = (UInt)getIntVal(locationDecoration->getLocation());
                    }
                    if (auto semanticDecor = input->findDecoration<IRSemanticDecoration>())
                    {
                        varLayoutBuilder.setSystemValueSemantic(semanticDecor->getSemanticName(), semanticDecor->getSemanticIndex());
                    }
                    else
                    {
                        fieldTypeLayout.addResourceUsage(LayoutResourceKind::VaryingInput, LayoutSize(1));
                        if (entryPointDecor->getProfile().getStage() == Stage::Fragment)
                        {
                            varLayoutBuilder.setUserSemantic("COLOR", inputVarIndex);
                        }
                        else if (entryPointDecor->getProfile().getStage() == Stage::Vertex)
                        {
                            varLayoutBuilder.setUserSemantic("VERTEX_IN_", inputVarIndex);
                        }
                        inputVarIndex++;
                    }
                    inputStructTypeLayoutBuilder.addField(key, varLayoutBuilder.build());
                    input->transferDecorationsTo(key);
                }
                auto paramTypeLayout = inputStructTypeLayoutBuilder.build();
                IRVarLayout::Builder paramVarLayoutBuilder(&builder, paramTypeLayout);
                paramLayout = paramVarLayoutBuilder.build();

                // Add an entry point parameter for all the inputs.
                auto firstBlock = entryPointFunc->getFirstBlock();
                builder.setInsertInto(firstBlock);
                auto inputParam = builder.emitParam(inputStructType);
                builder.addLayoutDecoration(inputParam, paramLayout);

                // Initialize all global variables.
                for (Index i = 0; i < inputVars.getCount(); i++)
                {
                    auto input = inputVars[i];
                    setInsertBeforeOrdinaryInst(&builder, firstBlock->getFirstOrdinaryInst());
                    auto inputType = cast<IRPtrTypeBase>(input->getDataType())->getValueType();
                    builder.emitStore(input,
                        builder.emitFieldExtract(inputType, inputParam, inputKeys[i]));
                    // Relate "global variable" to a "global parameter" for use later in compilation 
                    // to resolve a "global variable" shadowing a "global parameter" relationship.
                    builder.addGlobalVariableShadowingGlobalParameterDecoration(inputParam, input, inputKeys[i]);
                }

                // For each entry point, introduce a new parameter to represent each input parameter,
                // and return all outputs via a struct value.
                if (hasOutput)
                {
                    // If we have global outputs, the entry-point must not return anything itself.
                    if (as<IRFuncType>(entryPoint->getDataType())->getResultType()->getOp() != kIROp_VoidType)
                    {
                        context->getSink()->diagnose(entryPointFunc, Diagnostics::entryPointMustReturnVoidWhenGlobalOutputPresent);
                        continue;
                    }
                    builder.setInsertBefore(entryPointFunc);
                    resultType = builder.createStructType();
                    IRStructTypeLayout::Builder typeLayoutBuilder(&builder);
                    UInt outputVarIndex = 0;
                    for (auto output : outputVars)
                    {
                        auto key = builder.createStructKey();
                        if (auto nameHint = output->findDecoration<IRNameHintDecoration>())
                        {
                            builder.addNameHintDecoration(key, nameHint->getName());
                        }
                        auto ptrType = as<IRPtrTypeBase>(output->getDataType());
                        builder.createStructField(resultType, key, ptrType->getValueType());
                        IRTypeLayout::Builder fieldTypeLayout(&builder);
                        IRVarLayout::Builder varLayoutBuilder(&builder, fieldTypeLayout.build());
                        varLayoutBuilder.setStage(entryPointDecor->getProfile().getStage());
                        if (auto semanticDecor = output->findDecoration<IRSemanticDecoration>())
                        {
                            varLayoutBuilder.setSystemValueSemantic(semanticDecor->getSemanticName(), semanticDecor->getSemanticIndex());
                        }
                        else
                        {
                            fieldTypeLayout.addResourceUsage(LayoutResourceKind::VaryingOutput, LayoutSize(1));

                            if (auto locationDecoration = output->findDecoration<IRGLSLLocationDecoration>())
                            {
                                varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::VaryingOutput)->offset = (UInt)getIntVal(locationDecoration->getLocation());
                            }
                            if (entryPointDecor->getProfile().getStage() == Stage::Fragment)
                            {
                                varLayoutBuilder.setSystemValueSemantic("SV_TARGET", outputVarIndex);
                            }
                            else if (entryPointDecor->getProfile().getStage() == Stage::Vertex)
                            {
                                varLayoutBuilder.setUserSemantic("COLOR", outputVarIndex);
                            }
                            outputVarIndex++;
                        }
                        typeLayoutBuilder.addField(key, varLayoutBuilder.build());
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
                            auto resultVal = builder.emitMakeStruct(resultType, (UInt)fieldVals.getCount(), fieldVals.getBuffer());
                            builder.emitReturn(resultVal);
                            returnInst->removeAndDeallocate();
                        }
                    }
                }
                if (auto entryPointLayoutDecor = entryPointFunc->findDecoration<IRLayoutDecoration>())
                {
                    if (auto entryPointLayout = as<IREntryPointLayout>(entryPointLayoutDecor->getLayout()))
                    {
                        if (paramLayout)
                            builder.replaceOperand(entryPointLayout->getOperands(), paramLayout);
                        if (resultVarLayout)
                            builder.replaceOperand(entryPointLayout->getOperands() + 1, resultVarLayout);
                    }

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
        }

        // If we see a `GetWorkGroupSize` instruction, we should materialize it by replacing its uses with a constant
        // that represent the workgroup size of the calling entrypoint.
        // This is trivial if the `GetWorkGroupSize` instruction is used from a function called by one entry point.
        // If it is used in a place reachable from multiple entry points, we will introduce a global variable to represent
        // the workgroup size, and replace the uses with a load from the global variable.
        // We will assign the value of the global variable at the start of each entry point.
        //
        void materializeGetWorkGroupSize(IRModule* module, Dictionary<IRInst*, HashSet<IRFunc*>>& referenceGraph, IRInst* workgroupSizeInst)
        {
            IRBuilder builder(workgroupSizeInst);
            traverseUses(workgroupSizeInst, [&](IRUse* use)
                {
                    if (auto parentFunc = getParentFunc(use->getUser()))
                    {
                        auto referenceSet = referenceGraph.tryGetValue(parentFunc);
                        if (!referenceSet) 
                        	return;
                        if (referenceSet->getCount() == 1)
                        {
                            // If the function that uses the workgroup size is only used by one entry point,
                            // we can materialize the workgroup size by substituting the use with a constant.
                            auto entryPoint = *referenceSet->begin();
                            auto numthreadsDecor = entryPoint->findDecoration<IRNumThreadsDecoration>();
                            if (!numthreadsDecor)
                                return;
                            builder.setInsertBefore(use->getUser());
                            IRInst* values[] = {
                                numthreadsDecor->getExtentAlongAxis(0),
                                numthreadsDecor->getExtentAlongAxis(1),
                                numthreadsDecor->getExtentAlongAxis(2) };
                            auto workgroupSize = builder.emitMakeVector(builder.getVectorType(builder.getIntType(), 3),
                                3, values);
                            builder.replaceOperand(use, workgroupSize);
                        }
                    }
                });
            
            // If workgroupSizeInst still has uses, it means it is used by multiple entry points.
            // We need to introduce a global variable and assign value to it in each entry point.

            if (!workgroupSizeInst->hasUses())
                return;
            builder.setInsertBefore(workgroupSizeInst);
            auto globalVar = builder.createGlobalVar(workgroupSizeInst->getFullType());

            // Replace all remaining uses of the workgroupSize inst of a load from globalVar.
            traverseUses(workgroupSizeInst, [&](IRUse* use)
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
                    IRInst* args[] = {
                        numthreadsDecor->getExtentAlongAxis(0),
                        numthreadsDecor->getExtentAlongAxis(1),
                        numthreadsDecor->getExtentAlongAxis(2) };
                    auto workgroupSize = builder.emitMakeVector(
                        workgroupSizeInst->getFullType(), 3, args);
                    builder.emitStore(globalVar, workgroupSize);
                }
            }

            workgroupSizeInst->removeAndDeallocate();
        }
    };

    void translateGLSLGlobalVar(CodeGenContext* context, IRModule* module)
    {
        GlobalVarTranslationContext ctx;
        ctx.context = context;
        ctx.processModule(module);
    }
}
