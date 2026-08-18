//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#typealignment--the-operand-order-exception
//META: doc_section_digest=1b464e6340173025e21e0129aef42edde338d1c2b441ba933bd965108bf692da
//META: purpose=A TypeAlignment attribute reads the alignment from operand 0 and omits the layout-unit operand for the byte default, unlike the kind-first size and offset attributes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented exception to the kind-first convention. The struct
// is 48 bytes aligned to 16 and its `float` field is aligned to 4, so two
// distinct alignment attributes exist; each prints with a single operand whose
// value is the alignment itself. Reading operand 0 as a resource kind -- the
// habit `size` and `offset` teach -- would see 16 and 4 as kind enumerators,
// which is exactly the confusion this shape guards against.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct S
{
    float a;
    float4 b;
    int c;
}

ConstantBuffer<S> cb;
RWStructuredBuffer<float> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    buf[0] = cb.a + cb.b.x + float(cb.c);
}

// CHECK-DAG: = TypeAlignment(16 : Int)
// CHECK-DAG: = TypeAlignment(4 : Int)
// A byte-unit size still spells the kind first, for contrast.
// CHECK-DAG: = size(8 : Int, 48 : Int)
