#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif

RWStructuredBuffer<float > outputBuffer_0 : register(u0);

struct Tuple_0
{
    uint value0_0;
};

uint _S1(uint _S2)
{
    switch(_S2)
    {
    case 1U:
        {
            return 3U;
        }
    case 2U:
        {
            return 4U;
        }
    default:
        {
            return 0U;
        }
    }
}

float A_calc_0(float x_0)
{
    return x_0 * x_0 * x_0;
}

float B_calc_0(float x_1)
{
    return x_1 * x_1;
}

float _S3(uint _S4, float _S5)
{
    switch(_S4)
    {
    case 3U:
        {
            return A_calc_0(_S5);
        }
    case 4U:
        {
            return B_calc_0(_S5);
        }
    default:
        {
            return 0.0f;
        }
    }
}

float f_0(uint id_0, float x_2)
{
    Tuple_0 obj_0;
    if(id_0 == 0U)
    {
        obj_0.value0_0 = 1U;
    }
    else
    {
        obj_0.value0_0 = 2U;
    }
    return _S3(_S1(obj_0.value0_0), x_2);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DispatchThreadID)
{
    outputBuffer_0[int(0)] = f_0(0U, 1.0f);
    return;
}

