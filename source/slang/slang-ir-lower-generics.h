// slang-ir-lower-generics.h
#pragma once

namespace Slang
{
    struct IRModule;

    /// Lower generic and interface-based code to ordinary types and functions using
    /// dynamic dispatch mechanisms.
    void lowerGenerics(
        IRModule* module);

}
