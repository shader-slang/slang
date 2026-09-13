#include "slang-ir-lower-conditional-type.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
struct ConditionalTypeLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    struct LoweredConditionalTypeInfo
    {
        IRType* loweredType;
        bool hasValue;
    };
    Dictionary<IRConditionalType*, LoweredConditionalTypeInfo> loweredConditionalTypes;
    IRType* emptyStructType = nullptr;

    ConditionalTypeLoweringContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule)
    {
    }

    IRType* getEmptyStructType()
    {
        if (!emptyStructType)
        {
            IRBuilder builder(module);
            builder.setInsertInto(module->getModuleInst());
            auto emptyStruct = builder.createStructType();
            builder.addNameHintDecoration(
                emptyStruct,
                UnownedStringSlice("_slang_Conditional_empty"));
            emptyStructType = emptyStruct;
        }
        return emptyStructType;
    }

    void addToWorkList(IRInst* inst)
    {
        if (workListSet.contains(inst))
            return;
        workList.add(inst);
        workListSet.add(inst);
    }

    // Lower `condType` on first use and cache the result; return its lowering info, or null when it
    // cannot be lowered — either its own `hasValue` flag is not a literal, or a nested conditional
    // value type is itself unlowerable. A null return is always accompanied by a diagnostic,
    // reported at `useLoc` (the consuming inst's location, or the type's own from the eager walk).
    //
    // Resolving on demand — rather than assuming an earlier walk already recorded every conditional
    // type — keeps lowering independent of the order in which the worklist visits a type versus the
    // make/get insts that consume it.
    //
    // The returned pointer aliases storage inside `loweredConditionalTypes`, a dense map whose
    // values move on insertion; a caller must use it before any subsequent call that may insert
    // (including a nested resolve) and must not hold it across one. Every current caller
    // dereferences it immediately.
    LoweredConditionalTypeInfo* tryLowerConditionalType(
        IRConditionalType* condType,
        SourceLoc useLoc)
    {
        if (auto existing = loweredConditionalTypes.tryGetValue(condType))
            return existing;

        auto hasValueInst = condType->getHasValue();

        bool hasValue = false;
        bool resolved = false;

        if (auto boolLit = as<IRBoolLit>(hasValueInst))
        {
            hasValue = boolLit->getValue();
            resolved = true;
        }
        else if (auto intLit = as<IRIntLit>(hasValueInst))
        {
            hasValue = getIntVal(intLit) != 0;
            resolved = true;
        }

        if (!resolved)
        {
            // A non-literal `hasValue` cannot arise from valid input: by the time this pass runs
            // (after `finalizeSpecialization`), specialization has made every generic value
            // parameter concrete. One here is an upstream bug; `Conditional` has no
            // non-literal-flag representation on any backend, so emit a located diagnostic to fail
            // cleanly here.
            sink->diagnose(Diagnostics::Unexpected{
                .message = "Conditional<T,hasValue> reached code generation with a non-literal "
                           "hasValue flag",
                .location = useLoc.isValid() ? useLoc : condType->sourceLoc});
            return nullptr;
        }

        LoweredConditionalTypeInfo info;
        info.hasValue = hasValue;

        if (hasValue)
        {
            IRType* resolvedType = condType->getValueType();
            if (auto innerCond = as<IRConditionalType>(resolvedType))
            {
                // A nested conditional value type that cannot itself be lowered makes the outer
                // type unlowerable too; propagate the failure (the inner type is diagnosed by this
                // recursive call). Because the recursion fully lowers the inner type, one level of
                // unwrapping here handles arbitrary `Conditional<Conditional<...>>` depth — no loop
                // is needed — and it terminates because value-type nesting is finite and acyclic.
                auto innerInfo = tryLowerConditionalType(innerCond, useLoc);
                if (!innerInfo)
                    return nullptr;
                resolvedType = innerInfo->loweredType;
            }
            // A fully lowered type is never itself an `IRConditionalType`; the recursion above
            // guarantees it, and the terminal replacement loop plus downstream consumers rely on
            // it.
            SLANG_ASSERT(!as<IRConditionalType>(resolvedType));
            info.loweredType = resolvedType;
        }
        else
        {
            // Lower to a shared empty struct.
            info.loweredType = getEmptyStructType();
        }

        loweredConditionalTypes[condType] = info;
        return &loweredConditionalTypes[condType];
    }

    void processMakeConditionalValue(IRMakeConditionalValue* inst)
    {
        auto condType = as<IRConditionalType>(inst->getDataType());
        if (!condType)
            return;
        auto info = tryLowerConditionalType(condType, inst->sourceLoc);
        if (!info)
            return;

        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        if (info->hasValue)
        {
            inst->replaceUsesWith(inst->getValue());
        }
        else
        {
            auto emptyVal = builder.emitMakeStruct(info->loweredType, 0, nullptr);
            inst->replaceUsesWith(emptyVal);
        }
        inst->removeAndDeallocate();
    }

    void processGetConditionalValue(IRGetConditionalValue* inst)
    {
        auto condType = as<IRConditionalType>(inst->getConditionalOperand()->getDataType());
        if (!condType)
        {
            // Already lowered.
            auto operand = inst->getConditionalOperand();
            IRBuilder builder(module);
            builder.setInsertBefore(inst);
            if (operand->getDataType() == inst->getDataType())
                inst->replaceUsesWith(operand);
            else
                inst->replaceUsesWith(builder.getPoison(inst->getDataType()));
            inst->removeAndDeallocate();
            return;
        }
        auto info = tryLowerConditionalType(condType, inst->sourceLoc);
        if (!info)
            return;

        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        if (info->hasValue)
        {
            inst->replaceUsesWith(inst->getConditionalOperand());
        }
        else
        {
            auto poisonVal = builder.getPoison(inst->getDataType());
            inst->replaceUsesWith(poisonVal);
        }
        inst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_ConditionalType:
            // Eagerly lower and cache every conditional type, including those with no make/get
            // consumer to trigger on-demand lowering: the terminal replacement loop in
            // processModule rewrites each cached type wherever it still appears (field / parameter
            // / return positions), so a type reached only through those positions must be cached
            // here.
            tryLowerConditionalType(as<IRConditionalType>(inst), inst->sourceLoc);
            break;
        case kIROp_MakeConditionalValue:
            processMakeConditionalValue(as<IRMakeConditionalValue>(inst));
            break;
        case kIROp_GetConditionalValue:
            processGetConditionalValue(as<IRGetConditionalValue>(inst));
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();
            workList.removeLast();
            workListSet.remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Replace all conditional types with lowered types.
        for (const auto& [key, value] : loweredConditionalTypes)
            key->replaceUsesWith(value.loweredType);
    }
};

void lowerConditionalType(IRModule* module, DiagnosticSink* sink)
{
    ConditionalTypeLoweringContext context(module);
    context.sink = sink;
    context.processModule();
}
} // namespace Slang
