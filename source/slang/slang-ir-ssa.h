// slang-ir-ssa.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;
    bool constructSSA(IRModule* module, IRGlobalValueWithCode* globalVal);
    bool constructSSA(IRModule* module);
}
