//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=fdd06f0a3a11b54325a8474139094f83e3f6174898d99c25f5f4cdcfc864d79a
//META: purpose=The Func type opcode places the result type first, then parameter types in declaration order.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Func type opcode places the result type first, then parameter
// types in declaration order. The result type is the leading operand;
// the remaining operands are parameter types in declaration order." A
// helper `int helper(int v, float f)` surfaces as `Func(Int, Int,
// Float)` in the dump.

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
