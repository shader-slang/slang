//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#targetintrinsic-targetintrinsicdecoration
//META: doc_section_digest=cb8a8327f83d2f0346939069b3bffffc9403ae749340d6d2499128810dc798f5
//META: purpose=Calling a core-module built-in pulls in its [targetIntrinsic(capSet, "spelling")] decoration in the IR dump's core-module section.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc: "targetIntrinsic is the IR's encoding of 'this
// function is implemented by a target-specific spelling'. The two
// operands are a capability-set operand identifying the target(s)
// it applies to, and an IRStringLit operand carrying the
// target-language source code or instruction name." Calling
// `pow(x, y)` from user code forces the core-module declaration
// (and its targetIntrinsic decorations) into the dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float x = float(tid.x);
    buf[0] = pow(x, 2.0);
}

// CHECK: [targetIntrinsic(%{{[0-9]+}}, "pow")]
