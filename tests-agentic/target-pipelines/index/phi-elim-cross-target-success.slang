//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:33Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/target-pipelines/index.md#cross-target-comparison
//META: doc_section_digest=2189df77247b3c5793c79496940422c4d89b7ff54728e5b6402f4c854362eece
//META: purpose=Phi-elimination is gated per-target (register-allocation only for SPIR-V; default options for HLSL, Metal, WGSL, CUDA); a source with merging control flow still emits successfully on every backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc's cross-target comparison prose states that
// `eliminatePhis` runs with register-allocation enabled only for
// SPIR-V (`isKhronosTarget && emitSpirvDirectly`), and with default
// options for HLSL, Metal, WGSL, CUDA. The user-observable
// cross-cutting consequence is that an SSA-shaped source with an
// `if`-merge (i.e. a place a phi would have been introduced) still
// reaches successful text emission on every backend; the per-target
// gating is invisible from the surface.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v;
    if ((tid.x & 1u) != 0u)
        v = 11;
    else
        v = 22;
    outBuf[tid.x] = v;
}

// Every backend emits the merge value. We do not assert on the phi
// shape (it is target-specific and the index doc says only that the
// gating is per-target); we assert each backend successfully reached
// text-emit with the literal merge values present.
// CHECK_HLSL: 11
// CHECK_HLSL: 22
// CHECK_SPV: OpConstant {{.*}} 11
// CHECK_SPV: OpConstant {{.*}} 22
// CHECK_METAL: 11
// CHECK_METAL: 22
// CHECK_WGSL: 11
// CHECK_WGSL: 22
// CHECK_CUDA: 11
// CHECK_CUDA: 22
