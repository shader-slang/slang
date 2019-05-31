// slang-ir-specialize.h
#pragma once

namespace Slang
{
struct IRModule;

    /// Specialize generic and interface-based code to use concrete types.
void specializeModule(
    IRModule*   module);

}
