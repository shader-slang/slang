//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#getstringhash
//META: doc_section_digest=93e0a40b474b2be04ad15bdfb24b3f96cd0a29df6e929ab8f254fcc5f2c7154c
//META: purpose=The hash is resolved before code generation, so the string literal's bytes never reach the emitted target code on any text-emit backend.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the practical payoff the doc gives for hashing at compile time:
// reflection and capability code can key on a string without the backend
// carrying its bytes. The literal is a unique marker string, so a single
// CHECK-NOT per target is a complete statement about that target's whole
// output — if any backend deferred the hash to run time, the marker would have
// to appear somewhere in the generated source.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = getStringHash("fnv1a-fold-probe-9137");
}

//HLSL-NOT: fnv1a-fold-probe-9137
//HLSL: main

//GLSL-NOT: fnv1a-fold-probe-9137
//GLSL: void main

//SPV-NOT: fnv1a-fold-probe-9137
//SPV: OpEntryPoint

//METAL-NOT: fnv1a-fold-probe-9137
//METAL: main

//WGSL-NOT: fnv1a-fold-probe-9137
//WGSL: main

//CUDA-NOT: fnv1a-fold-probe-9137
//CUDA: main

//CPP-NOT: fnv1a-fold-probe-9137
//CPP: main
