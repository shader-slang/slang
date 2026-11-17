//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:34:02+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#existential-construction
//META: doc_section_digest=fc4731125d3e1fb116f0025ce0d4b2ecc5234e8d4fae162606e0795f4acc047e
//META: purpose=A buffer read of an interface-typed element whose interface has no conformance in the linkage stops code generation with E50100, "no type conformances found".
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the "load or buffer read of an interface value" site of the empty
// conformance set. `src[0]` yields an existential whose concrete type is
// unknown at compile time, so the type-flow pass collects the witness tables
// of `IFoo` that survived linking. No type conforms to `IFoo` here, the set
// is empty, and the pass reports E50100 at the dispatch.
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv-asm -entry main -stage compute

interface IFoo { int run(int x); };

uniform int inVal;
uniform RWStructuredBuffer<IFoo> src;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = src[0].run(inVal);
}

// CHECK: error[E50100]: no type conformances found
// CHECK: No type conformances are found for interface 'IFoo'.
