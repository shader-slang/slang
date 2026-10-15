//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute -wgsl-diagnostic warning:subgroup_uniformity -wgsl-diagnostic off:subgroup_uniformity -wgsl-diagnostic info:derivative_uniformity
//TEST:SIMPLE(filecheck=NONE): -target wgsl -entry computeMain -stage compute

// `-wgsl-diagnostic <severity>:<rule>` emits a global `diagnostic` directive. Directives come right
// after the `enable` lines, before prelude helpers such as `_slang_getInfinity`; for a repeated
// rule only the last severity is emitted.

// CHECK: enable subgroups;
// CHECK-NOT: diagnostic(warning, subgroup_uniformity);
// CHECK: diagnostic(off, subgroup_uniformity);
// CHECK-NEXT: diagnostic(info, derivative_uniformity);
// CHECK-NOT: diagnostic(
// CHECK: fn _slang_getInfinity
// CHECK: subgroupAdd(

// NONE-NOT: diagnostic(

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float value = 1.0 / 0.0;
    // A subgroup operation in non-uniform control flow: valid only with subgroup_uniformity off.
    if (outputBuffer[dispatchThreadID.x] > 0.5)
        value += WaveActiveSum(outputBuffer[dispatchThreadID.x]);
    outputBuffer[dispatchThreadID.x] = value;
}
