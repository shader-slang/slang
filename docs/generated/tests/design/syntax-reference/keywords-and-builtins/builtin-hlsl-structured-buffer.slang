//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=61eae4cb09aaf55ca13e1b5adcdbe1bcd8d876e194e57de10e1861ffeb9396c7
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
