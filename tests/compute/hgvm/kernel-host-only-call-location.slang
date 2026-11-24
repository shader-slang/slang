//TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan
//TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12

// A dispatched lambda that calls a host-only operation fails when its kernel is built. The error
// names the pipeline and points at the call in the user's source on every backend, rather than
// printing the compiler's internal representation of the host-only instruction.
//
// The E56050 error comes from the BACKSTOP `checkHostOnlyInstsInDeviceCode`. The capability
// system should reject this call first; when it does, this test should expect that error instead
// and the backstop can be removed.

import slang.runtime;
import slang.compute.buffer_utils;

[host]
void main()
{
    let values = RWStructuredBuffer<uint>::alloc(4);
    values.fill(0u);
    Grid(4u).dispatch(
        (uint i) =>
        {
            let scratch = RWStructuredBuffer<uint>::alloc(4);
            scratch[0] = i;
            values[i] = scratch[0];
        });
    print(readBack(values, 0));
}

// CHECK: result code = 1
// CHECK: HGVM failed to build the device code for compute pipeline '{{[^']+}}':
// CHECK: error[E56050]: host-only operation in device code
// CHECK: kernel-host-only-call-location.slang:23:
// CHECK-NOT: HGBCAsm
