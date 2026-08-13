//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=e40d6712a2443477b9dd87fd4146f5ac3852042d7d571c4f7aa5a1ef274fdbc1
//META: purpose=The `select(condition, trueValue, falseValue)` builtin lowers to the branch-free `select` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "select" notable-opcode note says `select` is the
// branch-free conditional, "an *expression* — it produces a value
// and does not affect control flow." The HLSL/Slang `select(c, x, y)`
// builtin is the surface form that lowers to the `select` opcode
// directly (the ternary `?:` lowers instead to short-circuit
// `ifElse` — recorded as a doc gap in README.md).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool c;
uniform int x;
uniform int y;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(c, x, y);
}

// IR-LABEL: func %main
// IR: select(%c, %x, %y)
