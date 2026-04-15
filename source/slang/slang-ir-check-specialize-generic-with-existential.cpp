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

// Recursively visit the entire module and mark IRSpecialize instructions that use an existential
// or interface type as a specialization argument. Recognized argument patterns include
// ExtractExistentialType, ExtractExistentialWitnessTable, MakeExistential, InterfaceType,
// TypeEqualityWitness whose first operand is an InterfaceType, and LookupWitnessMethod whose
// witness table is an ExtractExistentialWitnessTable (associated-type pattern, #9934).
// The actual diagnostic is emitted later by the typeflow or specialize passes.
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
                            IRBuilder builder(parent->getModule());
                            builder.addDecoration(
                                specialize,
                                kIROp_DisallowSpecializationWithExistentialsDecoration);
                            goto nextInst;
                        }
                    case kIROp_TypeEqualityWitness:
                        {
                            if (!as<IRInterfaceType>(specArg->getOperand(0)))
                                continue;

                            IRBuilder builder(parent->getModule());
                            builder.addDecoration(
                                specialize,
                                kIROp_DisallowSpecializationWithExistentialsDecoration);
                            goto nextInst;
                        }
                    case kIROp_LookupWitnessMethod:
                        {
                            // Associated type looked up from an existential witness table:
                            //   lookupWitness(extractExistentialWitnessTable(obj), key)
                            // This produces an existential-derived type that cannot be
                            // statically resolved for unconstrained generics (#9934).
                            auto lookupWitness = as<IRLookupWitnessMethod>(specArg);
                            if (lookupWitness->getWitnessTable()->getOp() !=
                                kIROp_ExtractExistentialWitnessTable)
                                continue;

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
