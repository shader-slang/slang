//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=12582dfa01eb81163b717e59e208875fece8e3cb827d3738a483bbc260d0c7d8
//META: purpose=`StructuredBuffer<T>` is an HLSL-flavored type supplied by `hlsl.meta.slang`; it survives into the emitted HLSL text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Use `StructuredBuffer<int>` for input and `RWStructuredBuffer<int>`
// for output. HLSL re-emits the `StructuredBuffer` token verbatim, and
// every other shader text target lowers it to that target's read-only
// storage form, so each prefix pins the target's own spelling of the
// read-only half of the pair: GLSL a `readonly buffer` block, SPIR-V a
// `NonWritable`-decorated `StructuredBuffer` block, Metal a `device`
// address-space pointer, WGSL a `var<storage, read>` array. Emitting
// the read-write `outBuf` from the same shader keeps the read-only
// decoration meaningful rather than incidental.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    //HLSL: StructuredBuffer
    outBuf[0] = inBuf[0] + 1;
}

//GLSL: readonly buffer StructuredBuffer_int_t_
//SPIRV-DAG: OpName %StructuredBuffer "StructuredBuffer"
//SPIRV-DAG: OpDecorate %inBuf NonWritable
//METAL: int device* inBuf{{.*}}{{\[\[}}buffer(0)
//WGSL: var<storage, read> inBuf{{.*}} : array<i32>
