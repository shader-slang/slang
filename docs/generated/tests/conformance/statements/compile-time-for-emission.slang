//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#compile-time-for-statement
//META: doc_section_digest=b1d04c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d0e1f2a3
//META: purpose=$for is fully unrolled in the emitted HLSL output (no loop structure, repeated body).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that $for emits as fully-unrolled code on every shader text target
// (no loop structure remains). The body assigns to arr[i]; each target's output
// should have three explicit constant-index assignments and no loop construct.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    int arr[3];
    $for(i in Range(0, 3))
    {
        arr[i] = i * 10;
    }
    outputBuffer[0] = arr[0];
    outputBuffer[1] = arr[1];
    outputBuffer[2] = arr[2];
}

// The emitted HLSL should contain the index values 0, 10, 20 from the unrolled assignments
// CHECK: 0
// CHECK: 10
// CHECK: 20

// GLSL emits three constant-index stores between the function open and the
// buffer writes, with no `for`/`while` construct in between.
// GLSL: void main()
// GLSL-NOT: for(
// GLSL-NOT: while(
// GLSL: arr_{{[0-9]+}}[0] = 0;
// GLSL: arr_{{[0-9]+}}[1] = 10;
// GLSL: arr_{{[0-9]+}}[2] = 20;

// METAL: {{\[\[}}kernel{{\]\]}} void computeMain
// METAL-NOT: for(
// METAL-NOT: while(
// METAL: arr_{{[0-9]+}}[int(0)] = int(0);
// METAL: arr_{{[0-9]+}}[int(1)] = int(10);
// METAL: arr_{{[0-9]+}}[int(2)] = int(20);

// WGSL: fn computeMain
// WGSL-NOT: for(
// WGSL-NOT: loop
// WGSL: arr_{{[0-9]+}}[i32(0)] = i32(0);
// WGSL: arr_{{[0-9]+}}[i32(1)] = i32(10);
// WGSL: arr_{{[0-9]+}}[i32(2)] = i32(20);

// In SPIR-V the unrolled form is three OpStore of the literal constants inside
// a single basic block; a loop would have produced an OpLoopMerge.
// SPIRV: %computeMain = OpFunction
// SPIRV-NOT: OpLoopMerge
// SPIRV: OpStore %{{[0-9]+}} %int_0
// SPIRV: OpStore %{{[0-9]+}} %int_10
// SPIRV: OpStore %{{[0-9]+}} %int_20
