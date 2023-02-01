// slang-ir-init-local-var.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;
    struct SharedIRBuilder;

    // Init local variables with default values if the variable isn't being initialized locally in
    // the same basic block.
    void initializeLocalVariables(SharedIRBuilder* sharedBuilder, IRGlobalValueWithCode* func);

}
