//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=A checker-range code (E30019) is rendered for a type-mismatch condition.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `type-mismatch` (30019) to its rendered code. The diag has
// a typed-name interpolation via ~expectedType:Type and
// ~actualType:QualType in the span message.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int4 v = int4(int(tid.x));
    bool b = v;
}

// CHECK: E30019
// CHECK: type mismatch
// CHECK: expected an expression of type 'bool'
