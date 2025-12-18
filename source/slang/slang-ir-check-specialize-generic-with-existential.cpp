// slang-ir-check-specialize-generic-with-existential.cpp
#include "slang-ir-check-specialize-generic-with-existential.h"

#include "core/slang-type-text-util.h"
#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

class DiagnosticSink;
struct IRModule;

// Recursively visit the entire module, and diagnose an error whenever an ExtractExistentialType is
// being used as a specialization argument to a generic function or type.
void addDecorationsForGenericsSpecializedWithExistentialsRec(IRInst* parent, DiagnosticSink* sink)
{
    if (auto code = as<IRGlobalValueWithCode>(parent))
    {
        for (auto block : code->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto specialize = as<IRSpecialize>(inst);
                if (!specialize)
                    continue;
                for (UInt i = 0; i < specialize->getArgCount(); i++)
                {
                    auto specArg = specialize->getArg(i);
                    switch (specArg->getOp())
                    {
                    case kIROp_InterfaceType:
                    case kIROp_ExtractExistentialType:
                    case kIROp_ExtractExistentialWitnessTable:
                    case kIROp_MakeExistential:
                        {
                            IRInst* specializationBase = specialize->getBase();
                            if (auto generic = as<IRGeneric>(specializationBase))
                                specializationBase = findInnerMostGenericReturnVal(generic);
                            if (auto lookupWitness =
                                    as<IRLookupWitnessMethod>(specialize->getBase()))
                                specializationBase = lookupWitness->getRequirementKey();
                            IRBuilder builder(parent->getModule());
                            builder.addDecoration(
                                specialize,
                                kIROp_DisallowSpecializationWithExistentialsDecoration);
                            goto nextInst;
                        }
                    }
                }
            nextInst:;
            }
        }
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
