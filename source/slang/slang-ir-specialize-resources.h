// slang-ir-specialize-resources.h
#pragma once

namespace Slang
{
    class BackEndCompileRequest;
    class TargetRequest;
    struct IRModule;
    struct IRType;

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
    bool specializeResourceParameters(
        BackEndCompileRequest* compileRequest,
        TargetRequest*  targetRequest,
        IRModule*       module);

    bool specializeResourceOutputs(
        BackEndCompileRequest*  compileRequest,
        TargetRequest*          targetRequest,
        IRModule*               module);

        /// Combined iterative passes of `specializeResourceParameters` and `specializeResourceOutputs`.
    bool specializeResourceUsage(
        BackEndCompileRequest* compileRequest, TargetRequest* targetRequest, IRModule* irModule);

    bool isIllegalGLSLParameterType(IRType* type);

}
