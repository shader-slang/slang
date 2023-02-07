// slang-ir-redundancy-removal.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRGlobalValueWithCode;

    bool removeRedundancy(IRModule* module);
    bool removeRedundancyInFunc(IRGlobalValueWithCode* func);

    bool eliminateRedundantLoadStore(IRGlobalValueWithCode* func);
}
