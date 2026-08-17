//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#entry-point-shape
//META: doc_section_digest=6840c04e5449d96826b3b300b825e82111aff2cddc60c36c5c9944110ef35fdf
//META: purpose=An amplification entry point is emitted with the `[[object]]` stage attribute and its dispatch payload parameter carries `[[payload]]` instead of a numbered binding slot.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two least-exercised members of the Metal stage-attribute set
// at once: the object (amplification) stage attribute, and the payload
// attribute that a mesh-pipeline payload parameter takes in place of a
// [[buffer(N)]] slot. Writing the payload and calling DispatchMesh keeps
// both live to emit; the CHECK matches the attributes as bare substrings
// because FileCheck reads a literal double bracket as a variable reference.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry ampMain -stage amplification

struct Payload { uint groupId; };
groupshared Payload payload;

[shader("amplification")]
[numthreads(1, 1, 1)]
void ampMain(uint3 tid : SV_DispatchThreadID)
{
    payload.groupId = tid.x;
    DispatchMesh(1, 1, 1, payload);
}

// CHECK: object{{.*}} void ampMain(
// CHECK-SAME: payload
