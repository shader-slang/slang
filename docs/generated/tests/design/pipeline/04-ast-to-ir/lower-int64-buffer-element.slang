//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A `RWStructuredBuffer<int64_t>` lowers so the element pointer returned by `rwstructuredBufferGetElementPtr` is `Ptr(Int64)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: storage axis — `int64_t` as the element type of a
// structured buffer. Observes the IR-side per-element `Ptr(...)`
// type produced by the lowered subscript-assignment.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int64_t a;
uniform RWStructuredBuffer<int64_t> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: Ptr(Int64){{.*}}= rwstructuredBufferGetElementPtr
