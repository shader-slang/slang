// slang-ir-hoist-constants.h
#pragma once

namespace Slang
{
struct IRModule;

    /// A (specialized) generic type may contain insts that computes compile-time constants defined within
    /// the type. We should hoist them to global scope so they can be SCCP'd when possible.
bool hoistConstants(
    IRModule*   module);

}
