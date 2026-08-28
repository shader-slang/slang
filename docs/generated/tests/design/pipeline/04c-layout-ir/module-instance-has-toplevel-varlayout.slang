//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=cb9b7a613d39760a93548dfbce14e96da09e14aa0f49313bd097e522bedd9206
//META: purpose=The global-scope var layout is attached to the module instance itself, appearing as a top-level varLayout instruction rather than a let-bound one.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the global-scope layout is anchored on the module
// instruction, not just on the individual globals. A decoration on the
// module instance prints without a `let` binding, so CHECK anchors
// `varLayout(` at the start of a line; every other varLayout in the block
// is the right-hand side of a `let`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// The module-instance varLayout is a top-level statement, no `let` binding.
// CHECK: {{^}}varLayout(
