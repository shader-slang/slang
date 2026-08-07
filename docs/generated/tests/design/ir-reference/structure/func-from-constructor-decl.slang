//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#functions-and-generics
//META: doc_section_digest=65157f975f1cbb6740bfae10984ea872d190878fcc32a62888fced45d662037d
//META: purpose=A ConstructorDecl lowers to a func just like an ordinary FuncDecl, carrying the constructed type as its Func(...) return type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__init` is not a special IR construct: it lowers through the
// same `func` opcode as any other function. CHECK finds the constructor's name
// hint and requires the next line to be a `func` header whose `Func(...)`
// result type returns the struct being constructed and takes the initializer's
// declared parameter.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

struct Boxed
{
    int v;
    __init(int x) { v = x * 2; }
}

[numthreads(1, 1, 1)]
void main()
{
    Boxed b = Boxed(seed);
    buf[0] = b.v;
}

// CHECK: [nameHint("Boxed.$init")]
// CHECK-NEXT: func %{{[A-Za-z0-9_]+}}	: Func(%Boxed, Int)
