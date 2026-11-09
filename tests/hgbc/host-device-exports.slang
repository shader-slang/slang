// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=INVALID): -target hlsl -entry hostHelper

// An unavailable host body must not introduce unresolved symbols into the device link.
extern uint externalHostHelper(uint value);

// An explicit export cannot make a host definition available to device programs.
[host]
export uint hostHelper(uint value)
{
    return externalHostHelper(value);
}

[host]
export uint genericHostHelper<T>(uint value)
{
    return hostHelper(value);
}

// Global uniform layout must survive the early device-program DCE.
uniform uint bias;
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    __target_switch
    {
    case hgbc:
        output[tid.x] = hostHelper(tid.x);
    default:
        output[tid.x] = bias + 42;
    }
}

// CHECK-NOT: hostHelper
// CHECK-NOT: genericHostHelper
// CHECK-NOT: externalHostHelper
// CHECK: computeMain
// CHECK-NOT: hostHelper
// CHECK-NOT: genericHostHelper
// CHECK-NOT: externalHostHelper
// INVALID: host function 'hostHelper' is referenced by a device program
