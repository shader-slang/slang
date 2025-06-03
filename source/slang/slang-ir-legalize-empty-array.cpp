#include "slang-ir-legalize-empty-array.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct EmptyArrayLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    Dictionary<IRInst*, IRInst*> replacements;

    EmptyArrayLoweringContext(IRModule* module)
        : module(module), workList(module), workListSet(module)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    bool isEmptyArray(IRType* t)
    {
        const auto lenLit = composeGetters<IRIntLit>(t, &IRArrayType::getElementCount);
        return lenLit ? getIntVal(lenLit) == 0 : false;
    };

    bool hasEmptyArrayType(IRInst* i) { return isEmptyArray(i->getDataType()); }

    bool hasEmptyArrayPtrType(IRInst* i)
    {
        const auto ptr = as<IRPtrTypeBase>(i->getDataType());
        return ptr && isEmptyArray(ptr->getValueType());
    }

    // If necessary, this returns a new instruction to replace the original one whose
    // operand is a 0-array.
    IRInst* getReplacement(IRInst* inst)
    {
        IRInst* replacement = nullptr;
        if (replacements.tryGetValue(inst, replacement))
            return replacement;

        IRBuilder builder(module);
        builder.setInsertBefore(inst);
        replacement = instMatch<IRInst*>(
            inst,
            nullptr,
            // The following match instructions which take a 0-sized array as an
            // operand and produces a result value for the inst.
            [&](IRGetElement* getElement)
            {
                const auto base = getElement->getBase();
                return hasEmptyArrayType(base) ? builder.emitUndefined(getElement->getDataType())
                                               : nullptr;
            },
            [&](IRGetElementPtr* gep)
            {
                const auto base = gep->getBase();
                return hasEmptyArrayPtrType(base)
                           ? builder.emitVar(as<IRPtrTypeBase>(gep->getDataType())->getValueType())
                           : nullptr;
            },
            // The following should match any instruction which can construct a 0-sized array.
            [&](IRMakeArray*) { return builder.getVoidValue(); },
            [&](IRMakeArrayFromElement*) { return builder.getVoidValue(); },
            // Otherwise if this is a 0-array type itself, replace it with
            // void type.
            [&](IRArrayType* arrayType)
            { return isEmptyArray(arrayType) ? builder.getVoidType() : nullptr; });

        // Sadly it's not really possible to catch missing cases here, as
        // there are heaps of instructions which don't do anything special
        // with vectors, but can take or return vector types, for example
        // arithmetic, IRGetElement, IRGetField etc...

        // If we did get a replacement, add that to our mapping and return
        // it, otherwise return the original (to maybe be updated later)
        if (replacement)
        {
            replacements.set(inst, replacement);

            // We need to add the replacement to the work list so that if it needs further
            // lowering, we will process it and register more entries into `replacements`.
            addToWorkList(replacement);
        }

        return replacement ? replacement : inst;
    }

    void processModule()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            // Run this inst through the replacer
            getReplacement(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Apply all replacements
        //
        // It's important to defer this as if we were updating things
        // on-the-fly we would be losing information about what was
        // actually a 0-array or not.
        for (const auto& [old, replacement] : replacements)
        {
            if (old != replacement)
            {
                old->replaceUsesWith(replacement);
                old->removeAndDeallocate();
            }
        }
    }
};

void legalizeEmptyArray(IRModule* module, DiagnosticSink* sink)
{
    EmptyArrayLoweringContext context(module);
    context.sink = sink;
    context.processModule();
}
} // namespace Slang
