// ir-specialize-resources.h
#pragma once

namespace Slang
{
    class CompileRequest;
    class TargetRequest;
    struct IRModule;

        /// Specialize calls to functions with resource-type parameters.
        ///
        /// For any function that has resource-type input parameters that
        /// would be invalid on the chosen target, this pass will rewrite
        /// any call sites that pass suitable arguments (e.g., direct
        /// references to global shader parameters) to instead call
        /// a specialized variant of the function that does not have
        /// those resource parameters (and instead, e.g, refers to the
        /// global shader parameters directly).
        ///
    void specializeResourceParameters(
        CompileRequest* compileRequest,
        TargetRequest*  targetRequest,
        IRModule*       module);
}
