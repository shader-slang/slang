// slang-ir-specialize-resources.cpp
#include "slang-ir-specialize-resources.h"

#include "slang-ir-specialize-function-call.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct ResourceParameterSpecializationCondition : FunctionCallSpecializeCondition
{
    // This pass is intended to specialize functions
    // with resource parameters to ensure that they are
    // legal for a given target.

    TargetRequest* targetRequest = nullptr;

    bool doesParamNeedSpecialization(IRParam* param)
    {
        // Whether or not a parameter needs specialization is really
        // a function of its type:
        //
        IRType* type = param->getDataType();

        // What's more, if a parameter of type `T` would need
        // specialization, then it seems clear that a parameter
        // of type "array of `T`" would also need specialization.
        // We will "unwrap" any outer arrays from the parameter
        // type before moving on, since they won't affect
        // our decision.
        //
        type = unwrapArray(type);

        // On all of our (current) targets, a function that
        // takes a `ConstantBuffer<T>` parameter requires
        // specialization. Surprisingly this includes DXIL
        // because dxc apparently does not treat `ConstantBuffer<T>`
        // as a first-class type.
        //
        if(as<IRUniformParameterGroupType>(type))
            return true;

        // For GL/Vulkan targets, we also need to specialize
        // any parameters that use structured or byte-addressed
        // buffers.
        //
        if( isKhronosTarget(targetRequest) )
        {
            if(as<IRHLSLStructuredBufferTypeBase>(type))
                return true;
            if(as<IRByteAddressBufferTypeBase>(type))
                return true;
        }

        // For now, we will not treat any other parameters as
        // needing specialization, even if they use resource
        // types like `Texure2D`, because these are allowed
        // as function parameters in both HLSL and GLSL.
        //
        // TODO: Eventually, if we start generating SPIR-V
        // directly rather than through glslang, we will need
        // to specialize *all* resource-type parameters
        // to follow the restrictions in the spec.
        //
        // TODO: We may want to perform more aggressive
        // specialization in general, especially insofar
        // as it could simplify the task of supporting
        // functions with resource-type outputs.

        return false;
    }
};

void specializeResourceParameters(
    BackEndCompileRequest* compileRequest,
    TargetRequest*  targetRequest,
    IRModule*       module)
{
    ResourceParameterSpecializationCondition condition;
    condition.targetRequest = targetRequest;
    specializeFunctionCalls(compileRequest, targetRequest, module, &condition);
}

} // namesapce Slang
