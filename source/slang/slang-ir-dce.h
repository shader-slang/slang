// slang-ir-dce.h
#pragma once

namespace Slang
{
    struct IRModule;

    struct IRDeadCodeEliminationOptions
    {
        bool keepExportsAlive = false;
        bool keepLayoutsAlive = false;
    };

        /// Eliminate "dead" code from the given IR module.
        ///
        /// This pass is primarily designed for flow-insensitive
        /// "global" dead code elimination (DCE), such as removing
        /// types that are unused, functions that are never called,
        /// etc.
        ///
    void eliminateDeadCode(
        IRModule*                           module,
        IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());
}
