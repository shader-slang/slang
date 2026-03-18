// slang-ir-check-specialize-generic-with-existential.cpp
#include "slang-ir-check-specialize-generic-with-existential.h"

#include "core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static void checkSpecializeInst(IRSpecialize* specialize, DiagnosticSink* sink)
{
    for (UInt i = 0; i < specialize->getArgCount(); i++)
    {
        auto specArg = specialize->getArg(i);
        switch (specArg->getOp())
        {
        case kIROp_InterfaceType:
            {
                // Explicit specialization with an interface type (e.g.
                // genericFunc<IFoo>(...)).  Emit E33180 here because
                // specializeModule will otherwise consume this Specialize inst
                // before typeflow-specialize can check the decoration.
                IRInst* specializationBase = specialize->getBase();
                if (auto generic = as<IRGeneric>(specializationBase))
                    specializationBase = findInnerMostGenericReturnVal(generic);
                String genericName = "<generic>";
                if (auto nameHint = specializationBase->findDecoration<IRNameHintDecoration>())
                    genericName = nameHint->getName();
                sink->diagnose(Diagnostics::CannotSpecializeGenericWithExistential{
                    .generic = genericName,
                    .location = specialize->sourceLoc});
                IRBuilder builder(specialize->getModule());
                builder.addDecoration(
                    specialize,
                    kIROp_DisallowSpecializationWithExistentialsDecoration);
                return;
            }
        case kIROp_ExtractExistentialType:
        case kIROp_ExtractExistentialWitnessTable:
        case kIROp_MakeExistential:
            {
                IRBuilder builder(specialize->getModule());
                builder.addDecoration(
                    specialize,
                    kIROp_DisallowSpecializationWithExistentialsDecoration);
                return;
            }
        }
    }
}

// Recursively visit the entire module, and diagnose an error whenever an existential type is
// being used as a specialization argument to a generic function or type.
// IRSpecialize insts can appear either inside function blocks (normal case) or at module scope
// (hoisted when all operands are global insts); both are handled by the recursive traversal.
void addDecorationsForGenericsSpecializedWithExistentialsRec(IRInst* parent, DiagnosticSink* sink)
{
    if (auto specialize = as<IRSpecialize>(parent))
    {
        checkSpecializeInst(specialize, sink);
        // Do not recurse into the operands of a Specialize inst.
        return;
    }

    for (auto childInst : parent->getDecorationsAndChildren())
    {
        addDecorationsForGenericsSpecializedWithExistentialsRec(childInst, sink);
    }
}

void addDecorationsForGenericsSpecializedWithExistentials(IRModule* module, DiagnosticSink* sink)
{
    addDecorationsForGenericsSpecializedWithExistentialsRec(module->getModuleInst(), sink);
}

} // namespace Slang
