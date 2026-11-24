//TEST:INTERPRET(hgvm,filecheck=D3D12): -api d3d12
//TEST:INTERPRET(hgvm,filecheck=WGPU): -api wgpu

// When slang-rhi cannot build a kernel's pipeline, the HGVM failure names the pipeline and
// includes the compiler output slang-rhi reported, so the user sees the source line at fault
// instead of only "failed to create an embedded compute pipeline".
//
// D3D12 has no float atomics without NVAPI, and WGSL has none at all, so the `InterlockedAdd`
// below fails when its dispatch builds the kernel.

import slang.runtime;
import slang.compute.buffer_utils;

[noinline]
float accumulate(RWStructuredBuffer<float> values)
{
    float previous;
    InterlockedAdd(values[0], 1.0, previous);
    return previous;
}

[host]
void main()
{
    let values = RWStructuredBuffer<float>::alloc(4);
    values.fill(0.0);
    Grid(4u).dispatch(
        (uint i) =>
        { accumulate(values); });
    print(readBack(values, 0));
}

// D3D12: result code = 1
// D3D12: HGVM failed to build the device code for compute pipeline '{{[^']+}}':
// D3D12: error[E55204]
// D3D12: pipeline-creation-error-messages.slang:18:
// D3D12: floating point atomic operation
// D3D12-NOT: HGVM failed

// WGPU: result code = 1
// WGPU: HGVM failed to build the device code for compute pipeline '{{[^']+}}':
// WGPU: [RHI error]
// WGPU-SAME: atomicAdd
// WGPU-NOT: HGVM failed
