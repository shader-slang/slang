//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#opcodes-with-no-producer-at-head
//META: doc_section_digest=2c918d154de28073fe1591f11040486a5209f2074ed76e85a18eb3170c74b205
//META: purpose=The __hasDiffTypeInfo constraint lowers to a void generic parameter satisfied by a void value, so no DiffTypeInfo inst is ever built.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies why `DiffTypeInfo` has a consumer but no producer. The AST side of
// the feature is real — the recognized differentiation interfaces are declared
// with a `__hasDiffTypeInfo(FType)` constraint — but the witness solving it is
// lowered to a void value, so the constraint contributes a plain generic
// parameter that every instantiation satisfies with `void_constant` instead of
// with a witness that would need a `DiffTypeInfo`. Differentiating `myfunc`
// instantiates one of those interfaces for it, and CHECK requires the
// resulting witness-table type to be `specialize(<generic>, %myfunc,
// void_constant)`: the second argument is the constrained type parameter and
// the third is the constraint. Anchoring on the user function name keeps the
// pattern out of the preamble, and the bracketing `CHECK-NOT`s require that no
// `DiffTypeInfo` inst exists in the section even though the feature was used.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[Differentiable]
float myfunc(float x)
{
    return x * x + x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = __fwd_diff(myfunc)(DifferentialPair<float>(a, d)).d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK-NOT: DiffTypeInfo
// CHECK: witness_table {{%[A-Za-z0-9_]+}} : witness_table_t(specialize({{%[A-Za-z0-9_]+}}, %myfunc, void_constant))(%myfunc)
// CHECK-NOT: DiffTypeInfo
// CHECK: {{^}}###{{$}}
