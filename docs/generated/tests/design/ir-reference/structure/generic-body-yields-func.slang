//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#generic
//META: doc_section_digest=b9f30006218f6bb377aa5edcb42ee78a54d51bcb2a455553e2fcca8e08a738f2
//META: purpose=A generic's single block ends with a return_val whose operand is the type-level result; for a generic function that result is the inner func value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the terminator contract that `findGenericReturnVal` relies on: the
// generic's block does not fall off the end or use a distinct yield opcode, it
// returns the computed type-level value. CHECK matches the inner `func %twice`
// definition and then requires `return_val(%twice)` — the same value the block
// just built — as the block's terminator.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int seed;

T twice<T : __BuiltinIntegerType>(T x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = twice<int>(seed);
}

// CHECK: func %twice	: Func(%T{{[A-Za-z0-9_]*}}, %T{{[A-Za-z0-9_]*}})
// CHECK: return_val(%twice)
