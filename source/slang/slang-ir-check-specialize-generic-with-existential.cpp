// slang-ir-check-specialize-generic-with-existential.cpp
#include "slang-ir-check-specialize-generic-with-existential.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

static void checkSpecializeInst(IRSpecialize* specialize, DiagnosticSink* sink)
{
    // Scan all args to determine if any existential-type args are present.
    // We must scan the whole list (not early-return) because a later arg may be
    // kIROp_InterfaceType even if an earlier arg is an Extract/MakeExistential.
    // kIROp_InterfaceType requires an eager E33180 diagnostic because specializeModule
    // may consume this Specialize inst before typeflow-specialize can check the
    // decoration.  Extract/MakeExistential args only need the decoration; typeflow-
    // specialize will emit E33180 for those cases as a safety net.
    bool shouldDecorate = false;
    bool shouldDiagnoseInterface = false;

    for (UInt i = 0; i < specialize->getArgCount(); i++)
    {
        auto specArg = specialize->getArg(i);
        switch (specArg->getOp())
        {
        case kIROp_InterfaceType:
            shouldDecorate = true;
            shouldDiagnoseInterface = true;
            break;
        case kIROp_ExtractExistentialType:
        case kIROp_ExtractExistentialWitnessTable:
        case kIROp_MakeExistential:
        case kIROp_MakeExistentialWithRTTI:
            shouldDecorate = true;
            break;
        }
    }

    if (!shouldDecorate)
        return;

    IRBuilder builder(specialize->getModule());
    builder.addDecoration(specialize, kIROp_DisallowSpecializationWithExistentialsDecoration);

    if (shouldDiagnoseInterface)
    {
        IRInst* specializationBase = specialize->getBase();
        if (auto generic = as<IRGeneric>(specializationBase))
            specializationBase = findInnerMostGenericReturnVal(generic);
        if (auto lookupWitness = as<IRLookupWitnessMethod>(specializationBase))
            specializationBase = lookupWitness->getRequirementKey();
        String genericName = "<generic>";
        if (auto nameHint = specializationBase->findDecoration<IRNameHintDecoration>())
            genericName = nameHint->getName();
        sink->diagnose(Diagnostics::CannotSpecializeGenericWithExistential{
            .generic = genericName,
            .location = specialize->sourceLoc});
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
