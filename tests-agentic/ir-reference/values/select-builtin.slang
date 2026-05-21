//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#select
//META: doc_section_digest=8d30dda9ed7d2748e6be00027e0ec0b5dd193f2f9d741727f3dc3eb6d99acea4
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
