// fallthrough-only-some-vars.slang
//
// Test that when multiple variables exist, only those actually used
// in fall-through target cases need IRVar treatment. Variables that
// are only used within a single case can use IRParam.
//
// This tests the optimization where we only use IRVar for variables
// that actually need it.

// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// CHECK:      267
// CHECK-NEXT: 25E
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int shared_var = 1;      // Used across fall-through - needs IRVar
    int case0_only = 100;    // Only used in case 0 - can use IRParam
    int case1_only = 200;    // Only used in case 1 - can use IRParam

    switch(val)
    {
    case 0:
        shared_var = 10;
        case0_only = case0_only * 2;  // 200
        // Fall through
    case 1:
        shared_var = shared_var + 5;
        case1_only = case1_only * 3;  // 600
        return shared_var + case1_only;

    default:
        return -1;
    }
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results:
    // tid=0: shared_var = 10+5 = 15, case1_only = 600, result = 615
    // tid=1: shared_var = 1+5 = 6, case1_only = 600, result = 606
    // tid=2: -1 (default)
    // tid=3: -1 (default)
    outputBuffer[tid] = test(int(tid));
}
