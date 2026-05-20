//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#generic
//META: doc_section_digest=de5860c73e271c909e6884da3649075d646d724d377f47e76f00e455a119f9c2
//META: purpose=A generic function lowers to a generic parent opcode containing a func child whose signature is parameterised by the type parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `T myGeneric<T : __BuiltinIntegerType>(T x) { ... }` should
// lower to a `generic %N : %M { ... func %myGeneric : Func(%T, %T) ... }`
// — the `generic` parent owns a single block whose body computes a
// type-level value (the `func` itself is the result of the generic).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

T myGeneric<T : __BuiltinIntegerType>(T x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = myGeneric<int>(7);
}

// CHECK: generic %{{[0-9]+}}	: {{.*}}
// CHECK: func %myGeneric	: Func(%T{{[A-Za-z0-9_]*}}, %T{{[A-Za-z0-9_]*}})
