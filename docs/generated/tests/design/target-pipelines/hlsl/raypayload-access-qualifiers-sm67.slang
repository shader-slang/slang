//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeraypayloadaccessqualifiersforhlsl
//META: doc_section_digest=684ef01d42bf0166bd93a05d657c09b3737cbd7bb9fbea4c5dc5d5f9d91f93ea
//META: purpose=At a DX shader model 6.7 profile, every member of a `[raypayload]` struct is emitted with both a `read(...)` and a `write(...)` qualifier — the missing side is filled structurally — while a 6.6 profile emits no qualifiers at all.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the payload-access-qualifier fill is keyed on the profile
// version and on the `[raypayload]` decoration itself, not on reachability
// from a `TraceRay` call site. The test declares a payload whose two fields
// each carry only one side of the qualifier pair and consumes it from a
// `closesthit` entry point with no trace call anywhere in the module. At
// `sm_6_7` the CHECK67 patterns assert each field ends up with both sides,
// with the author-written side left exactly as spelled and the missing side
// filled with the full stage list. At `sm_6_6` the qualifiers are not part
// of the language level, so the CHECK66-NOT patterns assert none are emitted
// — the contrast is what pins the behaviour to the profile predicate.

//TEST:SIMPLE(filecheck=CHECK67):-target hlsl -profile sm_6_7 -stage closesthit -entry main
//TEST:SIMPLE(filecheck=CHECK66):-target hlsl -profile sm_6_6 -stage closesthit -entry main

[raypayload]
struct HitPayload
{
    float3 readSide  : read(caller);
    float3 writeSide : write(caller);
};

[shader("closesthit")]
void main(inout HitPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.writeSide = payload.readSide;
}

// At SM 6.7 the struct carries the `[raypayload]` marker and both sides per field.
// CHECK67: struct [raypayload] HitPayload_{{[0-9]+}}
// The author-written `read(caller)` survives; the absent write side is filled.
// CHECK67-DAG: readSide_{{[0-9]+}} : read(caller) : write(
// The author-written `write(caller)` survives; the absent read side is filled.
// CHECK67-DAG: writeSide_{{[0-9]+}} : read({{.*}}) : write(caller);

// At SM 6.6 the pass does not run, so no access qualifiers are emitted.
// CHECK66: struct HitPayload_{{[0-9]+}}
// CHECK66-NOT: : read(
// CHECK66-NOT: : write(
