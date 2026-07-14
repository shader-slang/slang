//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:18:00Z
//META: source_commit=ef4bd0b0d91a213b52e333ce39dfe8b73efca3d2
//META: doc_ref=docs/generated/design/ir-reference/structure.md#generic
//META: doc_section_digest=afb5058aadcf477df5d62ea29df599323aa382ac64a993536ea1cf9976ac2133
//META: purpose=A generic's single block ends with a return_val (IRReturn) whose operand is the type-level result; for a generic function that result is the inner func value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a generic's body terminates in `return_val` carrying the
// type-level result. Lowering `twice<T>` produces a `generic` whose single
// block builds the inner `func %twice` and then returns it; the CHECKs
// assert the inner func definition is followed by `return_val(%twice)`,
// which is exactly the terminator-carries-the-result shape the catalog
// commits to (and what `findGenericReturnVal` reads back).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

T twice<T : __BuiltinIntegerType>(T x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = twice<int>(3);
}

// CHECK: func %twice	: Func(%T{{[A-Za-z0-9_]*}}, %T{{[A-Za-z0-9_]*}})
// CHECK: return_val(%twice)
