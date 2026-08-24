//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=An explicit `int(int8Value)` cast lowers to `intCast(%int8)` — narrow-to-wide signed integer conversion in the int-to-int family.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: explicit-cast axis — int8 to int. The existing
// `cast-uint-from-int-...` test covers `uint(int)`; this one
// pins the narrow-integer-to-int form.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t i8;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(i8);
}

// CHECK-LABEL: func %main
// CHECK: Int{{.*}}= intCast(%{{.*}})
