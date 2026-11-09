//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:26+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#uniformparametershadowvardecl
//META: doc_section_digest=c6fe74cd1fc3cdee3555e7000cd25e35c17eb37bbcdbb37d40feb743832a77a3
//META: purpose=Without the extended HLSL backwards-compatibility option no shadow variable is created, so assigning to a global uniform parameter in HLSL source is rejected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shadow variable is conditional on `-Gec`. The same program
// as the positive companion, compiled without the option, leaves the global
// uniform immutable, so the assignment reports E30011 (not an l-value).

//TEST:SIMPLE_EX(filecheck=CHECK,exit-code=-1):-lang hlsl docs/generated/tests/design/ast-reference/declarations/uniformparametershadowvardecl-assign-without-compat-rejected.slang -target hlsl -entry main -stage compute

uniform uint x;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    x = x + 5;
    outBuf[0] = x;
}

// CHECK: E30011
