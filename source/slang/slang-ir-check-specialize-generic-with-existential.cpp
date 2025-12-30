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
void addDecorationsForGenericsSpecializedWithExistentialsRec(IRInst* parent)
{
    if (auto code = as<IRGlobalValueWithCode>(parent))
    {
        for (auto block : code->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (auto specialize = as<IRSpecialize>(inst))
                {
                    // Extract the specialization target type.
                    auto targetType = specialize->getArg();

                    // Mark 
                    if (targetType->getOp() == kIROp_ExtractExistentialType)
                    {
                        IRBuilder builder(parent->getModule());
                        builder.addDecoration(
                            specialize,
                            kIROp_DisallowSpecializationWithExistentialsDecoration);
                    }
                }
            }
        }
    }

    for (auto childInst : parent->getDecorationsAndChildren())
    {
        addDecorationsForGenericsSpecializedWithExistentialsRec(childInst);
    }
}

void addDecorationsForGenericsSpecializedWithExistentials(IRModule* module)
{
    addDecorationsForGenericsSpecializedWithExistentialsRec(module->getModuleInst());
}

} // namespace Slang
