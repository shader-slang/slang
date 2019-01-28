// ir-entry-point-uniform.h
#pragma once

namespace Slang
{
struct IRModule;

    /// Move any uniform parameters of entry points to the global scope instead.
void moveEntryPointUniformParamsToGlobalScope(
    IRModule*   module);

}
