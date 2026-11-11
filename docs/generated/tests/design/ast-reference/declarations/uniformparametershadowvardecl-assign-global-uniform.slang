//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:26+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#uniformparametershadowvardecl
//META: doc_section_digest=c6fe74cd1fc3cdee3555e7000cd25e35c17eb37bbcdbb37d40feb743832a77a3
//META: purpose=For HLSL source with extended backwards compatibility, a global uniform parameter the program assigns to is accepted because a mutable shadow variable takes over its name.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `-Gec` HLSL input accepts an assignment to a global uniform
// parameter. The parser renames the original parameter and adds a shadow
// variable under the original name, so `x = x + 5` finds a mutable
// declaration. CHECK asserts the emitted HLSL initializes a mutable static
// copy from the constant-buffer field instead of rejecting the assignment.

//TEST:SIMPLE_EX(filecheck=CHECK):-lang hlsl docs/generated/tests/design/ast-reference/declarations/uniformparametershadowvardecl-assign-global-uniform.slang -Gec -target hlsl -entry main -stage compute

uniform uint x;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    x = x + 5;
    outBuf[0] = x;
}

// CHECK-DAG: static uint x_{{[0-9]+}} = x_{{[0-9]+}}_init();
// CHECK-DAG: return globalParams_0.x_0;
