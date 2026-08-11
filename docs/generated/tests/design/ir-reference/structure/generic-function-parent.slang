//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#generic
//META: doc_section_digest=b9f30006218f6bb377aa5edcb42ee78a54d51bcb2a455553e2fcca8e08a738f2
//META: purpose=A generic function lowers to a generic parent opcode containing a func child whose signature is parameterised by the type parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `GenericDecl` becomes a `generic` parent that is shaped like
// a function: it owns a block, and the inner `func` it builds is written in
// terms of the generic's type parameter. CHECK first anchors the `generic`
// header, then requires the inner `func %myGeneric` whose `Func(...)` type
// mentions the type-parameter value rather than a concrete type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int seed;

T myGeneric<T : __BuiltinIntegerType>(T x)
{
    return x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = myGeneric<int>(seed);
}

// CHECK: generic %{{[0-9]+}}	: {{.*}}
// CHECK: func %myGeneric	: Func(%T{{[A-Za-z0-9_]*}}, %T{{[A-Za-z0-9_]*}})
