// switch-fallthrough-dx11-vs-dx12.slang
//
// Test that generated HLSL for DXBC (DX11) eliminates fall-through, while 
// generated HLSL for DXIL (DX12) and plain HLSL preserves fall-through.
//
// For DXBC (DX11/FXC) - each case should have an explicit break, no fall-through
// For DXIL (DX12/DXC) or plain HLSL - Case 0 should fall through to case 1

// Plain HLSL output (assumes DXC) - fall-through should be preserved
// This test runs on all platforms
//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl -entry computeMain -stage compute

// DXBC (DX11) HLSL output - fall-through should be eliminated
// This test only runs on Windows with FXC
//TEST:SIMPLE(filecheck=CHECK-DXBC): -target dxbc -pass-through none -entry computeMain -stage compute

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

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    outputBuffer[tid] = test(int(tid));
}

