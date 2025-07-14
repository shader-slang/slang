#include "/home/runner/work/slang/slang/prelude/slang-cuda-prelude.h"


#line 3 "simple_test.slang"
extern "C" __global__ void test(RWStructuredBuffer<uint> output_0)
{

    *(&(output_0)[int(0)]) = 6U;
    return;
}

