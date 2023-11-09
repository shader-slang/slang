#include "slang-ir-translate-glsl-global-var.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
    struct GlobalVarTranslationContext
    {
        CodeGenContext* context;

        void processModule(IRModule* module)
        {
            List<IRInst*> outputVars;
            List<IRInst*> inputVars;
            List<IRInst*> entryPoints;
            for (auto inst : module->getGlobalInsts())
            {
                if (inst->findDecoration<IRGlobalOutputDecoration>())
                    outputVars.add(inst);
                if (inst->findDecoration<IRGlobalInputDecoration>())
                    inputVars.add(inst);
                if (inst->findDecoration<IREntryPointDecoration>())
                    entryPoints.add(inst);
            }

            IRBuilder builder(module);
            Dictionary<IRInst*, IRInst*> mapInputToGlobalVar;

            // For each input parameter, create a global var to represent it.
            // Replace all uses of the input with a `load` of the global var.
            for (auto input : inputVars)
            {
                auto type = input->getDataType();
                builder.setInsertBefore(input);
                auto globalVar = builder.createGlobalVar(type);
                mapInputToGlobalVar.add(input, globalVar);
                traverseUses(input, [&](IRUse* use)
                    {
                        if (as<IRAttr>(use->getUser()))
                            return;
                        if (as<IRDecoration>(use->getUser()))
                            return;
                        builder.setInsertBefore(use->getUser());
                        auto load = builder.emitLoad(globalVar);
                        builder.replaceOperand(use, load);
                    });
            }

            bool hasOutput = outputVars.getCount() != 0;

            for (auto entryPoint : entryPoints)
            {
                auto entryPointFunc = as<IRFunc>(entryPoint);
                if (!entryPointFunc)
                    continue;
             
                auto entryPointDecor = entryPointFunc->findDecoration<IREntryPointDecoration>();

                IRVarLayout* resultVarLayout = nullptr;
                IRVarLayout* paramLayout = nullptr;
                IRType* resultType = entryPointFunc->getResultType();

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
                        fieldTypeLayout.addResourceUsage(LayoutResourceKind::VaryingOutput, LayoutSize(1));
                        IRVarLayout::Builder varLayoutBuilder(&builder, fieldTypeLayout.build());
                        if (entryPointDecor->getProfile().getStage() == Stage::Fragment)
                        {
                            varLayoutBuilder.setSystemValueSemantic("SV_TARGET", outputVarIndex);
                        }
                        outputVarIndex++;
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
    };

    void translateGLSLGlobalVar(CodeGenContext* context, IRModule* module)
    {
        GlobalVarTranslationContext ctx;
        ctx.context = context;
        ctx.processModule(module);
    }
}
