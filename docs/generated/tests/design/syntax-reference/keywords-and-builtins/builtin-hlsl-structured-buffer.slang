//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=`StructuredBuffer<T>` is an HLSL-flavored type supplied by `hlsl.meta.slang`; it survives into the emitted HLSL text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Use `StructuredBuffer<int>` for input and `RWStructuredBuffer<int>`
// for output. The emit is checked for the `StructuredBuffer` token.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    //CHECK: StructuredBuffer
    outBuf[0] = inBuf[0] + 1;
}
