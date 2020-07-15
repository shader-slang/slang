// slang-ir-entry-point-uniform.h
#pragma once

#include "slang-compiler.h"

namespace Slang
{
struct IRModule;

struct CollectEntryPointUniformParamsOptions
{
    bool alwaysCreateCollectedParam;
};

    /// Collect entry point uniform parameters into a wrapper `struct` and/or buffer
void collectEntryPointUniformParams(
    IRModule*                                       module,
    CollectEntryPointUniformParamsOptions const&    options);

    /// Move any uniform parameters of entry points to the global scope instead.
void moveEntryPointUniformParamsToGlobalScope(
    IRModule*   module);

}
