//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#hlsl-compatibility-declarations
//META: doc_section_digest=2c1a661f3128523ca42f57d75ffd4649e625991cb2fdafb8ce1e9b1193d77d9d
//META: purpose=`CBufferDecl` parses as `'cbuffer' IDENT ':' Register '{' StructMember* '}'` with a `register(...)` clause.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CBufferDecl production including the optional
// `':' Register` clause. The HLSL back-end round-trips the construct,
// so asserting that the emitted text still declares a `cbuffer` bound
// to `register(b3)` — and that the member declared inside the braces
// survived as a field — proves the whole production was parsed rather
// than a prefix of it.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

cbuffer Params : register(b3)
{
    int scale;
}

RWStructuredBuffer<int> outputBuffer;

// CHECK-DAG: int scale_{{[0-9]+}};
// CHECK-DAG: cbuffer Params_{{[0-9]+}} : register(b3)
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = scale;
}
