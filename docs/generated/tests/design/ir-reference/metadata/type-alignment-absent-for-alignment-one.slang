//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#typealignment--the-operand-order-exception
//META: doc_section_digest=1b464e6340173025e21e0129aef42edde338d1c2b441ba933bd965108bf692da
//META: purpose=An absent TypeAlignment attribute encodes alignment 1, so the identity alignment is never emitted.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the alignment encoding. Layouts whose alignment
// is the identity (and layouts for a unit that occupies no space) get no
// attribute at all, because absence already means 1. The positive CHECK-DAG
// shows alignment attributes are being emitted in this module, and the
// CHECK-NOT asserts the redundant `TypeAlignment(1 : Int)` form never appears.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct S
{
    float4 b;
}

ConstantBuffer<S> cb;
RWStructuredBuffer<float> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    buf[0] = cb.b.x;
}

// CHECK-DAG: = TypeAlignment(16 : Int)
// CHECK-NOT: TypeAlignment(1 : Int)
