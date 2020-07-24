// slang-ir-specialize-arrays.h
#pragma once

namespace Slang
{
    class BackEndCompileRequest;
    class TargetRequest;
    struct IRModule;

    /// Specialize calls to functions that take parameters of
    /// `struct` type with array fields.
    ///
    /// For any function that has such input parameters, this pass
    /// will rewrite any call sites that pass suitable arguments
    /// (e.g., direct references to global shader parameters) to
    /// instead call a specialized variant of the function that does
    /// not have those array parameters, if the array can be referenced
    /// from global shader parameters directly.
    /// This is an optimization for GL/VK backend since the downstream
    /// compiler doesn't seem to optimize such code well.
    void specializeArrayParameters(
        BackEndCompileRequest* compileRequest,
        TargetRequest* targetRequest,
        IRModule* module);
}
