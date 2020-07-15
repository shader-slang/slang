// slang-ir-lower-generic-function.h
#pragma once

namespace Slang
{
    struct SharedGenericsLoweringContext;

    /// Lower generic and interface-based code to ordinary types and functions using
    /// dynamic dispatch mechanisms.
    void lowerGenericFunctions(
        SharedGenericsLoweringContext* sharedContext);

}
