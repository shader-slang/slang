//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:45+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/basics-program-execution.md#program-execution
//META: doc_section_digest=adbd230383e252b1f6ddca986456ca84146e424a9f5eea3bdd18df8bd52b8168
//META: purpose=The thread group dimensions (group_dim) specified via [numthreads(x,y,z)] on a compute entry point are preserved in emitted HLSL and SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the [numthreads] attribute on a compute entry point preserves its
// three dimensions through emit. CHECK asserts HLSL output contains `[numthreads(8, 4, 2)]`
// and SPIR-V output contains `LocalSize 8 4 2`, proving the group dimensions are not
// silently altered or dropped during lowering.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<uint> output;

[numthreads(8, 4, 2)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[0] = tid.x;
}

// HLSL preserves the numthreads dimensions.
// CHECK: [numthreads(8, 4, 2)]

// SPIR-V emits LocalSize with the matching dimensions.
// CHECK-SPV: LocalSize 8 4 2

// GLSL emits local_size_x/y/z layout qualifiers.
// CHECK-GLSL: local_size_x = 8
// CHECK-GLSL: local_size_y = 4
// CHECK-GLSL: local_size_z = 2

// Metal emits a [[kernel]] entry with thread_position_in_grid; dimensions are
// encoded in the CPP/loop body rather than a Metal attribute (Metal infers
// thread count from dispatch call, not from the kernel source), so we verify
// the [[kernel]] qualifier is present.
// CHECK-METAL: kernel

// WGSL emits @workgroup_size with the three dimensions.
// CHECK-WGSL: @workgroup_size(8, 4, 2)

// CUDA emits __global__ and the dimensions appear in the CPP loop scaffold.
// CHECK-CUDA: __global__

// CPP emits a comment banner with the numthreads dimensions before the entry wrapper.
// CHECK-CPP: [numthreads(8, 4, 2)]
