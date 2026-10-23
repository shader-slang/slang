//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute -wgsl-diagnostic warning:subgroup_uniformity -wgsl-diagnostic off:subgroup_uniformity -wgsl-diagnostic info:derivative_uniformity
//TEST:SIMPLE(filecheck=NONE): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute -wgsl-diagnostic off:subgroup_uniformity -wgsl-diagnostic warning:chromium.unreachable_code
//TEST:SIMPLE(filecheck=DOTTED): -target wgsl -entry computeMain -stage compute -wgsl-diagnostic warning:chromium.unreachable_code

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

// NONE: enable subgroups;
// NONE-NOT: diagnostic(
// NONE: fn _slang_getInfinity

// A rule may be two identifiers separated by a period.
// DOTTED: diagnostic(warning, chromium.unreachable_code);

// The bundled Tint compiles the module with the directives in place (both a plain and a dotted
// rule). It does not enforce subgroup_uniformity itself, so the run without the option would pass
// too; current Tint (e.g. Dawn's) rejects the shader below unless subgroup_uniformity is off.
// TINT: OpEntryPoint

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // The infinity literal makes the emitter add the `_slang_getInfinity` prelude helper, so the
    // checks above can confirm that the directives come before prelude declarations.
    float value = 1.0 / 0.0;
    // A subgroup operation in non-uniform control flow: current Tint accepts it only with
    // subgroup_uniformity off.
    if (outputBuffer[dispatchThreadID.x] > 0.5)
        value += WaveActiveSum(outputBuffer[dispatchThreadID.x]);
    outputBuffer[dispatchThreadID.x] = value;
}
