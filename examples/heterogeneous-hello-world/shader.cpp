#include "../../prelude/slang-cpp-prelude.h"


namespace { // anonymous 

#ifdef SLANG_PRELUDE_NAMESPACE
using namespace SLANG_PRELUDE_NAMESPACE;
#endif

struct KernelContext;


#line 13 "shader.slang"
struct UniformState
{

#line 4
    RWStructuredBuffer<float> ioBuffer_0;



};

struct KernelContext
{
    UniformState* uniformState;
    uint3 dispatchThreadID;
    uint3 groupID;
    uint3 groupDispatchThreadID;
    uint3 calcGroupThreadID() const 
    {
        uint3 v = { dispatchThreadID.x - groupDispatchThreadID.x, dispatchThreadID.y - groupDispatchThreadID.y, dispatchThreadID.z - groupDispatchThreadID.z };
        return v;
    }

#line 8
    void _computeMain()

    {

#line 10
        uint32_t tid_0 = dispatchThreadID.x;

        float i_0 = (uniformState->ioBuffer_0)[tid_0];
        bool _S1 = i_0 < 0.50000000000000000000f;

#line 13
        float _S2 = i_0 + i_0;

#line 13
        float _S3 = (F32_sqrt((i_0)));

#line 13
        float o_0 = _S1 ? _S2 : _S3;

        (uniformState->ioBuffer_0)[tid_0] = o_0;

#line 8
        return;
    }

};

} // anonymous

// [numthreads(4, 1, 1)]
SLANG_PRELUDE_EXPORT
void computeMain_Thread(ComputeThreadVaryingInput* varyingInput, void* params, void* uniformState)
{
    KernelContext context = {};
    context.uniformState = (UniformState*)uniformState;
    context.dispatchThreadID = {
        varyingInput->groupID.x * 4 + varyingInput->groupThreadID.x,
        varyingInput->groupID.y * 1 + varyingInput->groupThreadID.y,
        varyingInput->groupID.z * 1 + varyingInput->groupThreadID.z
    };
    context._computeMain();
}
// [numthreads(4, 1, 1)]
SLANG_PRELUDE_EXPORT
void computeMain_Group(ComputeVaryingInput* varyingInput, void* params, void* uniformState)
{
    KernelContext context = {};
    context.uniformState = (UniformState*)uniformState;
    const uint3 start = {
        varyingInput->startGroupID.x * 4,
        varyingInput->startGroupID.y * 1,
        varyingInput->startGroupID.z * 1
    };
    context.dispatchThreadID = start;
    for (uint32_t x = start.x; x < start.x + 4; ++x)
    {
        context.dispatchThreadID.x = x;
        context._computeMain();
    }
}
// [numthreads(4, 1, 1)]
SLANG_PRELUDE_EXPORT
void computeMain(ComputeVaryingInput* varyingInput, void* params, void* uniformState)
{
    KernelContext context = {};
    context.uniformState = (UniformState*)uniformState;
    const uint3 start = {
        varyingInput->startGroupID.x * 4,
        varyingInput->startGroupID.y * 1,
        varyingInput->startGroupID.z * 1
    };
    const uint3 end = {
        varyingInput->endGroupID.x * 4,
        varyingInput->endGroupID.y * 1,
        varyingInput->endGroupID.z * 1
    };
    for (uint32_t z = start.z; z < end.z; ++z)
    {
        context.dispatchThreadID.z = z;
        for (uint32_t y = start.y; y < end.y; ++y)
        {
            context.dispatchThreadID.y = y;
            for (uint32_t x = start.x; x < end.x; ++x)
            {
                context.dispatchThreadID.x = x;
                context._computeMain();
            }
        }
    }
}
