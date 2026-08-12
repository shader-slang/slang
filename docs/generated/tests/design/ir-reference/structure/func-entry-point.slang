//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#func
//META: doc_section_digest=8bdd4611fe4bde5f67b7b4377a4dac327c9defdb39a0f44cd421464470e7c1fd
//META: purpose=An entry-point function lowers to a func parent opcode whose Func(...) result type carries the signature.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function is a `func` parent inst whose signature lives on
// its own result type rather than in an operand list. The compute entry point
// `main` takes nothing and returns nothing, so CHECK asserts the dump header
// reads `func %main : Func(Void)` — a `Func(...)` result type with only the
// return type in it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = 1;
}

// CHECK: func %main	: Func(Void)
