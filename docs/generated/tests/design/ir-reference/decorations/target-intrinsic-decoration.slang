//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#targetintrinsic--targetintrinsicdecoration
//META: doc_section_digest=bcecad90942aef58e93b23cf4b28f81f01866d858ac796d92eb622fc5b09a36c
//META: purpose=Calling a core-module built-in pulls in its targetIntrinsic decoration, whose operands are a capability-set reference and the target-language spelling string.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two-operand form of `targetIntrinsic`: a capability-set
// operand naming the targets it applies to and a string carrying the
// target-language spelling. Calling `pow` links the core-module declaration
// into the module; the CHECK pins the string `"pow"` and wildcards the
// capability-set reference, whose inst id is unstable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float x = float(tid.x);
    buf[0] = pow(x, 2.0);
}

// CHECK: [targetIntrinsic(%{{[0-9]+}}, "pow")]
