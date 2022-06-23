// slang-ir-strip-cached-dict.h
#pragma once

namespace Slang
{
    struct IRModule;
    struct IRCall;

        /// Removes specialization dictionaries from module.
    void stripCachedDictionaries(IRModule* module);
}
