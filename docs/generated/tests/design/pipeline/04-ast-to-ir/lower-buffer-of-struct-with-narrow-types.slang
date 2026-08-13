//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A `RWStructuredBuffer<Narrow>` over a struct with narrow integer fields lowers so the per-element `Ptr(%Narrow)` is returned by `rwstructuredBufferGetElementPtr`, and the struct's narrow-typed `field(...)` rows survive.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: composition axis — RWStructuredBuffer of struct
// whose fields are narrow types. Verifies the buffer-element
// pointer type and the embedded struct's field IR types both
// surface in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Narrow { int8_t i8; uint16_t u16; }

uniform RWStructuredBuffer<Narrow> buf;
uniform int sel;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Narrow v = buf[sel];
    buf[0] = v;
}

// CHECK-LABEL: struct %Narrow
// CHECK: field({{.*}}, Int8)
// CHECK: field({{.*}}, UInt16)
// CHECK-LABEL: func %main
// CHECK: Ptr(%Narrow){{.*}}= rwstructuredBufferGetElementPtr
