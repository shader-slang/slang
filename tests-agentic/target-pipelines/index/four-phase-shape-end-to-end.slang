//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:33Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/target-pipelines/index.md#shared-shape
//META: doc_section_digest=f5436ca4de415916206a6431c15d6527da29b2d68cbec500330cd1a1e8b79acb
//META: purpose=The shared four-phase shape (link / specialize / target-legalize / emit) of linkAndOptimizeIR runs end-to-end for every text-emit target named by the index doc.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Shared shape" section names a four-phase decomposition that
// every per-target page obeys: Phase A link/entry-point prep, Phase B
// specialize/type-legalize, Phase C target-specific legalize + phi
// elim, Phase D emit. The cross-cutting observable is that a source
// that exercises something in each phase -- an entry point (A), a
// generic specialization (B), and a uniform parameter feeding the
// emitted output (C/D) -- reaches successful text emission on every
// text-emit backend without the user touching the pipeline.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

// A trivial generic; specialization in Phase B must collapse it.
T identity<T>(T x) { return x; }

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int t = identity<int>(int(tid.x));
    outBuf[tid.x] = t;
}

// Every backend produced output: the SourceEmitter / direct-emit path
// fired and the entry point made it through. We assert one robust
// per-target marker each; per-target details belong to peer bundles.

// CHECK_HLSL: void main
// CHECK_SPV: OpEntryPoint
// CHECK_METAL: kernel
// CHECK_WGSL: @compute
// CHECK_CUDA: __global__
