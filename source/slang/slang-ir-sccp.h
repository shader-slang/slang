// slang-ir-sccp.h
#pragma once

namespace Slang
{
    struct IRModule;

        /// Apply Sparse Conditional Constant Propagation (SCCP) to a module.
        ///
        /// This optimization replaces instructions that can only ever evaluate
        /// to a single (well-defined) value with that constant value, and
        /// also eliminates conditional branches where the condition will
        /// always evaluate to a constant (which can lead to entire blocks
        /// becoming dead code)
    void applySparseConditionalConstantPropagation(
        IRModule*       module);
}

