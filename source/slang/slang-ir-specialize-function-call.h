// slang-ir-specialize-function-call.h
#pragma once

namespace Slang
{
    class BackEndCompileRequest;
    class TargetRequest;
    struct IRModule;
    struct IRParam;

    class FunctionCallSpecializeCondition
    {
    public:
        virtual bool doesParamNeedSpecialization(IRParam* param) = 0;
    };


    /// Specialize calls to functions with certain type of parameters.
    ///
    /// For any function that has a specific type of input parameters
    /// this pass will rewrite its call sites that pass suitable arguments
    /// (e.g., direct references to global shader parameters) to instead call
    /// a specialized variant of the function that does not have
    /// those resource parameters (and instead, e.g, refers to the
    /// global shader parameters directly).
    ///
    void specializeFunctionCalls(
        BackEndCompileRequest* compileRequest,
        TargetRequest* targetRequest,
        IRModule* module,
        FunctionCallSpecializeCondition* condition);
}
