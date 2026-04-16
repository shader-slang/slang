// slang-ir-check-specialize-generic-with-existential.cpp
#include "slang-ir-check-specialize-generic-with-existential.h"

#include "slang-compiler.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

class DiagnosticSink;
struct IRModule;

// Returns true if `inst` transitively derives from an existential.
// Traces through LookupWitnessMethod chains to find existential roots,
// handling nested associated types (e.g. outer.Inner.Value) at any depth.
static bool isExistentialDerived(IRInst* inst, int depth = 0)
{
    if (depth > 16)
        return false;

    switch (inst->getOp())
    {
    case kIROp_InterfaceType:
    case kIROp_ExtractExistentialType:
    case kIROp_ExtractExistentialWitnessTable:
    case kIROp_MakeExistential:
        return true;
    case kIROp_TypeEqualityWitness:
        return as<IRInterfaceType>(inst->getOperand(0)) != nullptr;
    case kIROp_LookupWitnessMethod:
        return isExistentialDerived(
            as<IRLookupWitnessMethod>(inst)->getWitnessTable(),
            depth + 1);
    case kIROp_BuiltinCast:
        return isExistentialDerived(inst->getOperand(0), depth + 1);
    default:
        return false;
    }
}

// Recursively visit the entire module and mark IRSpecialize instructions that use an existential
// or interface type as a specialization argument.
// The actual diagnostic is emitted later by the typeflow or specialize passes.
void addDecorationsForGenericsSpecializedWithExistentialsRec(IRInst* parent, DiagnosticSink* sink)
{
    if (auto code = as<IRGlobalValueWithCode>(parent))
    {
        for (auto block : code->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                // The IRSpecialize may be a block-level child or the callee
                // of an IRCall (e.g. `call specialize(generic, arg)(...)`)
                IRSpecialize* specialize = as<IRSpecialize>(inst);
                if (!specialize)
                {
                    if (auto call = as<IRCall>(inst))
                        specialize = as<IRSpecialize>(call->getCallee());
                }
                if (!specialize)
                    continue;
                for (UInt i = 0; i < specialize->getArgCount(); i++)
                {
                    auto specArg = specialize->getArg(i);
                    if (isExistentialDerived(specArg))
                    {
                        IRBuilder builder(parent->getModule());
                        builder.addDecoration(
                            specialize,
                            kIROp_DisallowSpecializationWithExistentialsDecoration);
                        goto nextInst;
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
