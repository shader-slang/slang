//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#layout-family
//META: doc_section_digest=8133b77a147a20aa12fcc20641167e9fc2f42707469a60ef2b36207749f54a95
//META: purpose=A structTypeLayout carries one structFieldLayout attribute per field, and those attributes appear in field declaration order.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the per-field attribute tail and its ordering. `struct S` has three
// fields declared a, b, c; the CHECK captures the five operands of the
// `structTypeLayout` (two layout attributes followed by three field
// attributes) and then resolves each captured field operand by name. Because
// the capture fixes the position, matching `%a` on the third operand, `%b` on
// the fourth and `%c` on the fifth is what proves declaration order.

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

// CHECK: = structTypeLayout(%{{[0-9]+}}, %{{[0-9]+}}, %[[FA:[0-9]+]], %[[FB:[0-9]+]], %[[FC:[0-9]+]])
// CHECK-DAG: %[[FA]]{{.*}}= structFieldLayout(%a,
// CHECK-DAG: %[[FB]]{{.*}}= structFieldLayout(%b,
// CHECK-DAG: %[[FC]]{{.*}}= structFieldLayout(%c,
