// fallthrough-transitive-chain.slang
//
// Test transitive fall-through chains where a variable is assigned in an
// early case and used in a later case, skipping intermediate cases.
// This tests that the fall-through analysis correctly handles variables
// that span multiple cases in a chain.
//
// Specifically tests:
// - Variable 'a' assigned in case 0, NOT used in case 1, used in case 2
// - Variable 'b' assigned in case 1, NOT used in case 2, used in case 3
//
// This ensures the analysis correctly propagates variable tracking across
// multiple fall-through boundaries, not just immediate neighbors.
//
// DX11 and WGSL don't support fall-through natively, but the restructure
// pass legalizes fall-through by inlining code, so tests should still pass.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx11 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -mtl -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu

// tid=0: 131 = 0x83
// tid=1: 122 = 0x7A
// tid=2: 102 = 0x66
// tid=3: 101 = 0x65
// tid=4: -1 = 0xFFFFFFFF
// tid=5: -1 = 0xFFFFFFFF
// CHECK:      83
// CHECK-NEXT: 7A
// CHECK-NEXT: 66
// CHECK-NEXT: 65
// CHECK-NEXT: FFFFFFFF
// CHECK-NEXT: FFFFFFFF

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int a = 1;   // Assigned in case 0, used in case 2 (skips case 1)
    int b = 1;   // Assigned in case 1, used in case 3 (skips case 2)
    int c = 100;

    switch(val)
    {
    case 0:
        a = 10;  // Set a, NOT used until case 2
        // Fall through
    case 1:
        b = 20;  // Set b, NOT used until case 3
        c = c + 1;
        // Fall through
    case 2:
        c = c + a;  // Use 'a' which was set 2 cases ago
        // Fall through
    case 3:
        c = c + b;  // Use 'b' which was set 2 cases ago
        return c;

    default:
        return -1;
    }
}

[shader("compute")]
[numthreads(6, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    // Expected results (a=1, b=1, c=100 initially):
    //
    // tid=0 (enters at case 0):
    //   case 0: a = 10
    //   case 1: b = 20, c = 101
    //   case 2: c = 101 + 10 = 111  (a from case 0)
    //   case 3: c = 111 + 20 = 131  (b from case 1)
    //   return 131
    //
    // tid=1 (enters at case 1):
    //   case 1: b = 20, c = 101
    //   case 2: c = 101 + 1 = 102   (a unchanged = 1)
    //   case 3: c = 102 + 20 = 122  (b from case 1)
    //   return 122
    //
    // tid=2 (enters at case 2):
    //   case 2: c = 100 + 1 = 101   (a unchanged = 1)
    //   case 3: c = 101 + 1 = 102   (b unchanged = 1)
    //   return 102
    //
    // tid=3 (enters at case 3):
    //   case 3: c = 100 + 1 = 101   (b unchanged = 1)
    //   return 101
    //
    // tid=4, tid=5 (enters at default):
    //   return -1
    //
    outputBuffer[tid] = test(int(tid));
}
