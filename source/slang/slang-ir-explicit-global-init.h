// slang-ir-explicit-global-init.h
#pragma once

namespace Slang
{
struct IRModule;

    /// Move initialization logic off of global variables and onto each entry point
void moveGlobalVarInitializationToEntryPoints(
    IRModule* module);
}
