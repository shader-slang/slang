#pragma once

#include "slang-ir.h"

namespace Slang
{
    // The metal backend does not support global variables or parameters.
    // To workaround this restriction, we use this pass to wrap all the
    // global scope variables in a context type, and pass that context
    // type as the first parameter to all functions.

    void wrapGlobalScopeInContextType(IRModule* module);

}
