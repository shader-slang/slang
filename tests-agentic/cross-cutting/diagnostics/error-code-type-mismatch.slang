//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: assigning a vector to a bool variable renders E30019 with the documented "type mismatch in expression" message and interpolated expected/actual types.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `type-mismatch` (30019) to its rendered code. The diag has
// a typed-name interpolation via ~expectedType:Type and
// ~actualType:QualType in the span message.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int4 v = int4(int(tid.x));
    bool b = v;
}

// CHECK: E30019
// CHECK: type mismatch
// CHECK: expected an expression of type 'bool'
