//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#what-this-module-is-not
//META: doc_section_digest=205da7f598ad3d1643fab8a6fb17b6a8f761797e8d8d6da06cb330e073ca2298
//META: purpose=Linking pulls the existing layout module into its symbol set, so layout decorations absent pre-link become visible on the linked global and entry point.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the transition itself rather than either end of it. The
// CHECK-NOTs pin the pre-link region as layout-free and the CHECKs after
// the first post-link label find `EntryPointLayout(` plus a `[layout(...)]`
// on the global_param. Since no pre-link pass attaches those, their
// appearance is the observable evidence that link merged the layout module.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-NOT: [layout(
// CHECK-NOT: EntryPointLayout(
// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CHECK: EntryPointLayout(
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: global_param
