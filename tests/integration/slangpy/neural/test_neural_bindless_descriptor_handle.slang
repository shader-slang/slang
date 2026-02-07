// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Neural integration smoke test for bindless DescriptorHandle style resources.
// We import `neural` to ensure experimental module compilation works alongside bindless.

import neural;

struct BufferInfo
{
    StructuredBuffer<float>.Handle ro_buffer;
    RWStructuredBuffer<float>.Handle rw_buffer;
    uint offset;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(
    uint3 tid : SV_DispatchThreadID,
    StructuredBuffer<BufferInfo> buffer_infos,
    RWStructuredBuffer<float> results)
{
    uint index = tid.x;
    BufferInfo info = buffer_infos[index];

    float value = info.ro_buffer[info.offset];
    info.rw_buffer[info.offset] = value + 100.0;
    results[index] = value;
}
