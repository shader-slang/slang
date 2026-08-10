//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=8133b77a147a20aa12fcc20641167e9fc2f42707469a60ef2b36207749f54a95
//META: purpose=A pointer-typed field lowers to a ptrTypeLayout that stores only attributes, deliberately not a pointee layout.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a pointer layout keeps no reference to the layout of what it points
// at. A `float*` field inside a constant buffer produces `ptrTypeLayout`; both
// of its operands are captured and each is shown to be an attribute (the
// 8-byte pointer size and its 8-byte alignment). If a pointee layout were
// stored it would have to occupy one of those two operand slots.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct S
{
    float* p;
}

ConstantBuffer<S> cb;
RWStructuredBuffer<float> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    buf[0] = *cb.p;
}

// CHECK: = ptrTypeLayout(%[[SZ:[0-9]+]], %[[AL:[0-9]+]])
// CHECK: %[[AL]]{{.*}}= TypeAlignment(8 : Int)
// CHECK: %[[SZ]]{{.*}}= size(8 : Int, 8 : Int)
