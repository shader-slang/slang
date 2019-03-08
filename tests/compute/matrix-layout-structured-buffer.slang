// matrix-layout-structured-buffer.slang

// This test is set up to confirm that `StructuredBuffer` types are
// always laid out column-major by fxc/dxc, even when row-major layout has been
// requested globally.
//
// This behavior should be considered a bug in Slang because either:
//
//	1. we should report reflection layout information that acknowledges this behavior, or
// 	2. we should alter our HLSL output passed to fxc/dxc to provide consistent
//     behavior that matches our reflection data.
//
// For now this test exists to document the situation. It's output can/should
// be updated if we decide to fix the underlying problem by taking option (2).
//

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -xslang -matrix-layout-column-major


//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23], stride=48):dxbinding(0),glbinding(0)
RWStructuredBuffer<int3x4> gMatrices;

int test(int val)
{
    int N = 256;

    int tmp = 0;

    tmp = tmp*N + gMatrices[val%2][(val  )%3][(val  )%4];
    tmp = tmp*N + gMatrices[val%2][(val+1)%3][(val  )%4];
    tmp = tmp*N + gMatrices[val%2][(val  )%3][(val+1)%4];
    tmp = tmp*N + val;
    tmp = tmp + 0x80000000;

    return tmp;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):dxbinding(0),glbinding(1),out
RWStructuredBuffer<int> buffer;

[numthreads(12, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    int val = tid;
    val = test(val);

    buffer[tid] = val;
}
