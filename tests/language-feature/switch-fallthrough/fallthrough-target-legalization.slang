// HLSL output for DXC - fall-through should be preserved
//TEST:SIMPLE(filecheck=CHECK-HLSL6): -target hlsl -profile cs_6_0 -entry computeMain -stage compute

// HLSL output for FXC - fall-through should be eliminated
//TEST:SIMPLE(filecheck=CHECK-HLSL5): -target hlsl -profile cs_5_1 -pass-through none -entry computeMain -stage compute

// WGSL output - fall-through should be eliminated (WGSL doesn't support fall-through)
//TEST:SIMPLE(filecheck=CHECK-WGSL): -target wgsl -entry computeMain -stage compute

// Metal output - fall-through should be preserved (Metal supports fall-through)
//TEST:SIMPLE(filecheck=CHECK-METAL): -target metal -entry computeMain -stage compute

// HLSL SM 6.0+ should have fall-through - case 0 does NOT have break before case 1
// Warning should NOT be emitted for DXC (supports fall-through)
// CHECK-HLSL6-NOT: warning 41026
// CHECK-HLSL6: case int(0):
// CHECK-HLSL6: x_{{.*}} = int(100)
// CHECK-HLSL6-NOT: break
// CHECK-HLSL6: case int(1):

// HLSL SM 5.x should NOT have fall-through - case 0 has break before case 1
// Warning should be emitted for FXC (doesn't support fall-through)
// CHECK-HLSL5: warning 41026
// CHECK-HLSL5: case int(0):
// CHECK-HLSL5: x_{{.*}} = int(100)
// CHECK-HLSL5: break
// CHECK-HLSL5: case int(1):

// WGSL should NOT have fall-through - case 0 has break before case 1
// Warning should be emitted for WGSL (doesn't support fall-through)
// CHECK-WGSL: warning 41026
// CHECK-WGSL: case i32(0):
// CHECK-WGSL: x_{{.*}} = i32(100)
// CHECK-WGSL: break
// CHECK-WGSL: case i32(1):

// Metal should have fall-through - case 0 does NOT have break before case 1
// No warning should be emitted for Metal (supports fall-through)
// CHECK-METAL-NOT: warning 41026
// CHECK-METAL: case int(0):
// CHECK-METAL: x_{{.*}} = int(100)
// CHECK-METAL-NOT: break
// CHECK-METAL: case int(1):

RWStructuredBuffer<int> outputBuffer;

int test(int val)
{
    int result = 0;
    int x = 10;

    switch(val)
    {
    case 0:
        x = 100;
        // Fall through to case 1 (no break)
    case 1:
        result = x + val;
        break;
    case 2:
        result = 200;
        break;
    default:
        result = -1;
        break;
    }

    return result;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    outputBuffer[tid] = test(int(tid));
}
