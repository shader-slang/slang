// ir-existential.h
#pragma once

namespace Slang
{
    struct IRModule;

        /// Simplify code that makes use of existential types.
    void simplifyExistentialTypes(
        IRModule*   module);
}

