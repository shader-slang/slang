// slang-ir-ssa.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;
    struct IRInst;
    struct SharedIRBuilder;
    bool constructSSA(IRModule* module, IRGlobalValueWithCode* globalVal);
    bool constructSSA(SharedIRBuilder* sharedBuilder, IRGlobalValueWithCode* globalVal);
    bool constructSSA(IRModule* module);
    bool constructSSA(IRInst* globalVal);
}
