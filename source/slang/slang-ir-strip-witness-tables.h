// slang-ir-strip-witness-tables.cpp
#pragma once

namespace Slang
{
struct IRModule;

    /// Strip the contents of all witness table instructions from the given IR `module`
void stripWitnessTables(IRModule* module);

    /// Remove [KeepAlive] decorations from witness tables.
void unpinWitnessTables(IRModule* module);
}
