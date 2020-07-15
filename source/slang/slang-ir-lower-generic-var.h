// slang-ir-lower-generic-var.h
#pragma once

namespace Slang
{
    struct SharedGenericsLoweringContext;

    /// Lower load and stores of generic local variables into raw pointer operations.
    void lowerGenericVar(
        SharedGenericsLoweringContext* sharedContext);

}
