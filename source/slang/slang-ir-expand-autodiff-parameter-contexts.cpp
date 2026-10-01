#include "slang-ir-expand-autodiff-parameter-contexts.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
// Expose captured primal parameters as separate arguments to internal derivative functions.
// Autodiff deliberately packages these values into a parameter context, which can itself be a
// field of the full backward context. Passing that package as one aggregate can make downstream
// CUDA or Metal compilation materialize copies even when only some captured parameters are needed.
//
// Consider this example, written as pseudocode before aggregate parameters become references:
//
//     struct Settings { float scale; float unused[8]; };
//     struct SavedValues { float intermediate; };
//     struct ParameterContext { Settings settings; float x; }; // Marked parameter context.
//     struct FullContext { SavedValues saved; ParameterContext parameters; };
//
//     void propagate(FullContext context, inout float dx, float seed)
//     {
//         dx += context.saved.intermediate * context.parameters.x * seed;
//     }
//     propagate(FullContext(saved, ParameterContext(settings, x)), dx, seed);
//
// This pass changes the signature and every direct call together:
//
//     void propagate(SavedValues saved, Settings settings, float x, inout float dx, float seed)
//     {
//         auto context = FullContext(saved, ParameterContext(settings, x));
//         dx += context.saved.intermediate * context.parameters.x * seed;
//     }
//     propagate(saved, settings, x, dx, seed);
//
// Reconstructing the context preserves any uses of the whole value. Subsequent simplification
// folds field accesses through that reconstruction, exposing the independent arguments to later
// optimization without requiring the downstream compiler to inline the entire derivative first.
// Settings and SavedValues remain aggregates: only marked parameter contexts and the enclosing
// structs on paths to them are expanded. Exported signatures and pointer/ref parameters stay
// intact.
struct ExpandAutodiffParameterContextsContext
{
    IRModule* module;
    IRBuilder builder;
    Dictionary<IRType*, bool> containsParameterContextCache;

    ExpandAutodiffParameterContextsContext(IRModule* module)
        : module(module), builder(module)
    {
    }

    // Find a marked parameter context without unwrapping pointer or rate-qualified types.
    // Recursing through enclosing structs also handles the full context in the example above;
    // stopping at unmarked subtrees keeps original primal aggregates and saved values intact.
    bool containsParameterContext(IRType* type)
    {
        auto structType = as<IRStructType, IRDynamicCastBehavior::NoUnwrap>(type);
        if (!structType)
            return false;
        if (auto cached = containsParameterContextCache.tryGetValue(type))
            return *cached;
        bool result =
            structType->findDecoration<IRAutodiffParameterContextTypeDecoration>() != nullptr;
        for (auto field : structType->getFields())
            result |= containsParameterContext(field->getFieldType());
        containsParameterContextCache.add(type, result);
        return result;
    }

    // Only direct internal calls have a signature that this pass can update completely.
    bool canExpandFunction(IRFunc* func, List<IRCall*>& calls)
    {
        if (!func->isDefinition())
            return false;
        for (auto decoration : func->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_PublicDecoration:
            case kIROp_ExportDecoration:
            case kIROp_HLSLExportDecoration:
            case kIROp_DownstreamModuleExportDecoration:
            case kIROp_ExternCppDecoration:
            case kIROp_ExternCDecoration:
            case kIROp_DllExportDecoration:
            case kIROp_CudaDeviceExportDecoration:
            case kIROp_PyExportDecoration:
            case kIROp_EntryPointDecoration:
            case kIROp_CudaKernelDecoration:
            case kIROp_AutoPyBindCudaDecoration:
            case kIROp_TargetIntrinsicDecoration:
                return false;
            default:
                break;
            }
        }
        for (auto block : func->getBlocks())
            for (auto inst : block->getChildren())
                if (as<IRGenericAsm>(inst))
                    return false;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (as<IRDecoration>(user) || as<IRCompilerDictionaryValue>(user) ||
                as<IRCompilerDictionaryEntry>(user))
                continue;
            auto call = as<IRCall>(user);
            if (!call || call->getCalleeUse() != use)
                return false;
            calls.add(call);
        }
        return true;
    }

    // Reconstruct the original value for any whole-context uses. Field-only uses fold away during
    // ordinary simplification, while legitimate aggregate reads retain their original semantics.
    IRInst* expandParameter(IRType* type, IRParam* oldParam, Index& parameterCount)
    {
        if (!containsParameterContext(type))
        {
            auto param = builder.createParam(type);
            param->sourceLoc = oldParam->sourceLoc;
            param->insertBefore(oldParam);
            parameterCount++;
            return param;
        }
        List<IRInst*> fields;
        for (auto field : cast<IRStructType>(type)->getFields())
            fields.add(expandParameter(field->getFieldType(), oldParam, parameterCount));
        return builder.emitMakeStruct(type, fields);
    }

    // Decompose call arguments along exactly the same context paths as the callee parameters.
    void expandArgument(IRType* type, IRInst* value, List<IRInst*>& args)
    {
        if (!containsParameterContext(type))
        {
            args.add(value);
            return;
        }
        for (auto field : cast<IRStructType>(type)->getFields())
        {
            auto fieldValue =
                builder.emitFieldExtract(field->getFieldType(), value, field->getKey());
            expandArgument(field->getFieldType(), fieldValue, args);
        }
    }

    // A reconstructed aggregate is now a local debug value, while unchanged parameters keep their
    // argument identity at the updated index.
    void updateDebugArgumentIndices(IRFunc* func, List<Index> const& newIndices)
    {
        for (auto block : func->getBlocks())
        {
            for (auto inst = block->getFirstInst(); inst;)
            {
                auto next = inst->getNextInst();
                if (auto debugVar = as<IRDebugVar>(inst))
                {
                    if (auto index = as<IRIntLit>(debugVar->getArgIndex()))
                    {
                        auto oldIndex = index->getValue();
                        if (oldIndex >= 0 && oldIndex < newIndices.getCount())
                        {
                            auto newIndex = newIndices[(Index)oldIndex];
                            builder.setInsertBefore(debugVar);
                            if (newIndex >= 0)
                                debugVar->setArgIndex(
                                    builder.getIntValue(index->getFullType(), newIndex));
                            else
                            {
                                IRInst* operands[] = {
                                    debugVar->getSource(),
                                    debugVar->getLine(),
                                    debugVar->getCol()};
                                auto localVar = builder.emitIntrinsicInst(
                                    debugVar->getFullType(),
                                    kIROp_DebugVar,
                                    3,
                                    operands);
                                localVar->sourceLoc = debugVar->sourceLoc;
                                debugVar->transferDecorationsTo(localVar);
                                debugVar->replaceUsesWith(localVar);
                                debugVar->removeAndDeallocate();
                            }
                        }
                    }
                }
                inst = next;
            }
        }
    }

    void processFunction(IRFunc* func)
    {
        List<IRCall*> calls;
        if (!canExpandFunction(func, calls))
            return;

        List<IRType*> expandedTypes;
        List<IRParam*> oldParams;
        bool anyExpanded = false;
        for (auto param : func->getParams())
        {
            // Pointer, ref, inout, and rate-qualified parameters retain their calling convention.
            auto type = param->getFullType();
            bool expand = containsParameterContext(type);
            expandedTypes.add(expand ? type : nullptr);
            oldParams.add(param);
            anyExpanded |= expand;
        }
        if (!anyExpanded)
            return;

        List<Index> newIndices;
        Index parameterCount = 0;
        builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
        for (Index i = 0; i < oldParams.getCount(); i++)
        {
            auto param = oldParams[i];
            if (auto type = expandedTypes[i])
            {
                newIndices.add(-1);
                auto value = expandParameter(type, param, parameterCount);
                param->replaceUsesWith(value);
                param->removeAndDeallocate();
            }
            else
                newIndices.add(parameterCount++);
        }
        fixUpFuncType(func);

        for (auto call : calls)
        {
            builder.setInsertBefore(call);
            List<IRInst*> args;
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                if (auto type = expandedTypes[i])
                    expandArgument(type, call->getArg(i), args);
                else
                    args.add(call->getArg(i));
            }
            auto newCall = builder.emitCallInst(call->getFullType(), func, args);
            newCall->sourceLoc = call->sourceLoc;
            call->transferDecorationsTo(newCall);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }
        updateDebugArgumentIndices(func, newIndices);
        module->invalidateAnalysisForInst(func);
    }

    void processModule()
    {
        for (auto inst : module->getGlobalInsts())
            if (auto func = as<IRFunc>(inst))
                processFunction(func);
    }
};

void expandAutodiffParameterContexts(IRModule* module)
{
    ExpandAutodiffParameterContextsContext context(module);
    context.processModule();
}
} // namespace Slang
