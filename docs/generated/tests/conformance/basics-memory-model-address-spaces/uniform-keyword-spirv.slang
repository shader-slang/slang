//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=The `uniform` keyword on entry-point parameters also belongs to the Uniform address space; SPIRV emits it in a PushConstant block (single-element uniform parameters collapse to push-constant blocks on SPIRV).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `uniform` keyword on an entry-point parameter also belongs to the Uniform
// address space. Small `uniform` parameters are packed into a PushConstant block on SPIRV;
// CHECK asserts the assembly declares an `OpVariable ... PushConstant`, distinguishing this
// path from the `ConstantBuffer<T>` form tested in uniform-constant-buffer-spirv.slang.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

[numthreads(4, 1, 1)]
void computeMain(uint3           tid    : SV_DispatchThreadID,
                 uniform float   scale,
                 RWStructuredBuffer<float> output)
{
    output[tid.x] = float(tid.x) * scale;
}

// The uniform parameter is expected to land in a Uniform or PushConstant block
// (both are "Uniform address space" semantics: all threads, constant over dispatch).
// CHECK: OpVariable {{.*}} PushConstant
// Metal maps the uniform entry-point parameter to a constant* pointer (constant address space).
// METAL: constant* entryPointParams_
// WGSL maps the uniform entry-point parameter to var<uniform>.
// WGSL: var<uniform> entryPointParams_
