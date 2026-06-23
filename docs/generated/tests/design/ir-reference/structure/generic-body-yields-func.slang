//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#generic
//META: doc_section_digest=de5860c73e271c909e6884da3649075d646d724d377f47e76f00e455a119f9c2
//META: purpose=A generic body produces a func value as its yield result; the LOWER-TO-IR dump shows this as a return_val(%func) on the generic's single block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc, a generic body "ends with `yield` (not `return_val`)
// whose operand is the result of the type-level computation". The
// LOWER-TO-IR dump records this as the `func` being followed by a
// `return_val(%func)` on the generic's outer single block. (The
// underlying terminator opcode is `yield`; the dump prints it as
// `return_val` at this stage — note in `README.md` as a doc gap.)

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
