//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9f3194ddc2631c231ae0863cc8c3961c0a5eef6db41560bb17b1114e6c0cf6aa
//META: purpose=Boundary: a Slang struct with three heterogeneous fields (int, float, uint) is emitted as a target-native struct declaration on every C-like text target and as an OpTypeStruct on SPIR-V.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of the shared CLikeSourceEmitter struct-emit path:
// a multi-field struct stresses the field ordering and the per-field
// type-printer. (The existing single-int-field test in this bundle
// covers the trivial shape; this widens it to three different
// scalar types.) WGSL spells the struct fields differently — name
// first then type — so we just confirm the struct keyword.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct Vertex
{
    int idx;
    float weight;
    uint flags;
};

RWStructuredBuffer<Vertex> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Vertex v;
    v.idx = int(tid.x);
    v.weight = 0.5;
    v.flags = 0xFFu;
    outBuf[tid.x] = v;
}

// HLSL: struct Vertex_{{[0-9]+}}
// GLSL: struct Vertex_{{[0-9]+}}
// SPIRV: OpTypeStruct
// METAL: struct Vertex_{{[0-9]+}}
// WGSL: struct Vertex_{{[0-9]+}}
// CUDA: struct Vertex_{{[0-9]+}}
// CPP: struct Vertex_{{[0-9]+}}
