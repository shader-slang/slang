//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=1e957ad40dedfd3c52a2ed8eb2c7dd1be52c049778109f377a9197486f43c5df
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
