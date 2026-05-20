//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9bd773e205e5aa520ee4875c2a6aebc4a9d9aabba8cfac9d3b66489e9488c043
//META: purpose=The shared CLikeSourceEmitter base produces a `struct` declaration on every C-like text target (HLSL, GLSL, Metal, CUDA, C++) for a user struct. SPIR-V emits the same type as `OpTypeStruct`. WGSL emits a `struct` block.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The textual backends share most machinery via CLikeSourceEmitter
// (slang-emit-c-like.{h,cpp}). A consequence is that a user struct
// declared in source produces a recognizable `struct` declaration on
// each C-like text target, and the SPIR-V backend (which does not
// derive from CLikeSourceEmitter) emits the same type via
// `OpTypeStruct`. WGSL has its own `struct` syntax. The struct is
// kept alive by an RWStructuredBuffer<Pair> write that forces the
// type through legalization and emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct Pair
{
    int x;
    float y;
};

RWStructuredBuffer<Pair> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p;
    p.x = int(tid.x);
    p.y = float(tid.y);
    outBuf[tid.x] = p;
}

// HLSL: struct Pair_{{[0-9]+}}
// GLSL: struct Pair_{{[0-9]+}}
// SPIRV: OpTypeStruct
// METAL: struct Pair_{{[0-9]+}}
// WGSL: struct Pair_{{[0-9]+}}
// CUDA: struct Pair_{{[0-9]+}}
// CPP: struct Pair_{{[0-9]+}}
