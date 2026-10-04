// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Performance corpus: compare default/fast PTX using the commands in NVVM RESULTS.
// Runtime inputs prevent folding. These operations occur in Falcor sampling/material code.
StructuredBuffer<float4> inputs;
RWStructuredBuffer<float4> outputs;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    float4 value = inputs[id.x];
    float s;
    float c;
    sincos(value.x, s, c);
    outputs[id.x] = float4(s, c, log(value.y), pow(value.y, value.z));
}
