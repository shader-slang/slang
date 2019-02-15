// ir-dce.h
#pragma once

namespace Slang
{
    class BackEndCompileRequest;
    struct IRModule;

        /// Eliminate "dead" code from the given IR module.
        ///
        /// This pass is primarily designed for flow-insensitive
        /// "global" dead code elimination (DCE), such as removing
        /// types that are unused, functions that are never called,
        /// etc.
        ///
    void eliminateDeadCode(
        BackEndCompileRequest*  compileRequest,
        IRModule*               module);
}
