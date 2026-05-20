//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#func
//META: doc_section_digest=021d788619e4517ac1c3910bae98363f85bfa77ed12c6b9a50b7bd8e1f64f627
//META: purpose=The Func type opcode places the result type first, then parameter types in declaration order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "`Func`" note says
// "`Func(resultType, paramTypes...)` is the IR's function-type
// encoding. The result type is the leading operand; the remaining
// operands are parameter types in declaration order." A helper
// `int helper(int v, float f)` surfaces as `Func(Int, Int, Float)`
// in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float b;
uniform RWStructuredBuffer<int> buf;

int helper(int v, float f) { return v + int(f); }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a, b);
}

// IR-LABEL: func %helper
// IR-SAME: Func(Int, Int, Float)
