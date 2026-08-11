//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=The core-module `reinterpret<T, U>(value)` function lowers to the one-operand `reinterpret` opcode, which keeps the bit pattern and changes only the type tag.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `reinterpret` is a distinct opcode from `bitCast` and
// that it is reachable from ordinary Slang source (there is no
// `ReinterpretExpr` AST node — the core-module function is the only
// surface). A `uniform float` is reinterpreted as `uint` and stored, so
// nothing folds and nothing is dead. CHECK asserts a single-operand
// `reinterpret(%f)` whose result line is typed `UInt`, which is only
// produced if the call reached the `__intrinsic_op` route.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float f;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = reinterpret<uint, float>(f);
}

// IR-LABEL: func %main
// IR: : UInt	= reinterpret(%f)
