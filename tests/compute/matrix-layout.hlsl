// dxc-matrix-layout.hlsl

// This test tries to ensure that a `row_major` layout default specified on
// the command line makes it through to affect code generation on all targets.
// The test was created because it was found that released versions of dxc
// were ignoring the `#pragma pack_matrix` directive.

// This has a compatibility issue on Windows 10.0.10586 on Dx12 - dxcompiler will crash (can remove form tests with -exclude compatibility-issue)

//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -xslang -matrix-layout-row-major
//TEST(compute,compatibility-issue):COMPARE_COMPUTE_EX:-slang -compute -dx12 -use-dxil -xslang -matrix-layout-row-major

// Not testing on Vulkan because of lack of support
// for integer matrices in GLSL. Slang needs to
// supporting lowering of such matrix before we
// can run this test.
//
//NO_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -xslang -matrix-layout-row-major

struct S
{
    int3x4 a;
    int    b;
};

//TEST_INPUT:cbuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24]):dxbinding(0),glbinding(0)
cbuffer C0
{
    S s;
};

//TEST_INPUT:cbuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24]):dxbinding(1),glbinding(1)
cbuffer C1
{
    column_major
    int3x4 cc;
    int    dd;
};

int test(int val)
{
    int N = 256;

    // Note: using `val %3` here instead of `val %4` in order
    // to work around a code generation issue in dxc.
    //
    int a = s.a[val / 4][val % 3];
    int b = s.b;

    int c = cc[val / 4][val % 4];
    int d = dd;

    return ((a*N + b) * N + c) * N + d;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):dxbinding(0),glbinding(2),out
RWStructuredBuffer<int> buffer;

[numthreads(12, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    int val = tid;
    val = test(val);

    buffer[tid] = val;
}
