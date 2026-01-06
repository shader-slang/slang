// fallthrough-target-legalization.slang
//
// Test that generated output for targets that don't support fall-through
// (DXBC/DX11, WGSL) eliminates fall-through, while targets that do support
// it (DXIL/DX12, plain HLSL) preserve fall-through.

// Plain HLSL output (assumes DXC) - fall-through should be preserved
// This test runs on all platforms
//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl -entry computeMain -stage compute

// DXBC (DX11) HLSL output - fall-through should be eliminated
// This test only runs on Windows with FXC
//TEST:SIMPLE(filecheck=CHECK-DXBC): -target dxbc -pass-through none -entry computeMain -stage compute

// WGSL output - fall-through should be eliminated (WGSL doesn't support fall-through)
//TEST:SIMPLE(filecheck=CHECK-WGSL): -target wgsl -entry computeMain -stage compute

// Metal output - fall-through should be preserved (Metal supports fall-through)
//TEST:SIMPLE(filecheck=CHECK-METAL): -target metal -entry computeMain -stage compute

// HLSL should have fall-through - case 0 does NOT have break before case 1
// CHECK-HLSL: case int(0):
// CHECK-HLSL: x_{{.*}} = int(100)
// CHECK-HLSL-NOT: break
// CHECK-HLSL: case int(1):

// DXBC should NOT have fall-through - case 0 has break before case 1
// CHECK-DXBC: case int(0):
// CHECK-DXBC: x_{{.*}} = int(100)
// CHECK-DXBC: break
// CHECK-DXBC: case int(1):

// WGSL should NOT have fall-through - case 0 has break before case 1
// CHECK-WGSL: case i32(0):
// CHECK-WGSL: x_{{.*}} = i32(100)
// CHECK-WGSL: break
// CHECK-WGSL: case i32(1):

// Metal should have fall-through - case 0 does NOT have break before case 1
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

