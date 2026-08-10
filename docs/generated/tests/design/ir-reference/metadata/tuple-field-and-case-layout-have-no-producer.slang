//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#tuplefieldlayout-and-caselayout
//META: doc_section_digest=1b1b95a05054a92661513c3c0fbf09d1389ad761014d04f6bae616ed38737743
//META: purpose=The dormant tuple and union layout opcodes (tupleTypeLayout, tupleFieldLayout, caseLayout) have no producer in the compiler, so none of them appears in any IR dump.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the dormancy of the tuple and union layout opcodes whose builders
// have no caller.
// The module deliberately mixes a `Tuple` member (the closest surface to a
// tuple field layout) with an interface-typed member (the closest surface to a
// per-case layout), so both would-be producers are given their best chance.
// The positive CHECK-DAG proves layout lowering ran; the two CHECK-NOTs assert
// the attributes are still absent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface ILight
{
    float getIntensity();
}

struct S
{
    Tuple<float, int> t;
    ILight light;
}

ConstantBuffer<S> cb;
RWStructuredBuffer<float> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    buf[0] = cb.t._0 + float(cb.t._1);
}

// CHECK-DAG: = structTypeLayout(
// CHECK-NOT: tupleTypeLayout
// CHECK-NOT: tupleFieldLayout
// CHECK-NOT: caseLayout
