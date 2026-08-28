//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=cb9b7a613d39760a93548dfbce14e96da09e14aa0f49313bd097e522bedd9206
//META: purpose=With no parameter-group wrapping, the global scope's type layout is a plain structTypeLayout instruction built after the per-global-parameter loop.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unwrapped branch of the global-scope type layout: when no
// constant buffer wraps the global scope, the module-level layout is a
// plain struct layout. CHECK asserts a `structTypeLayout(` instruction is
// present before the entry-point function, which is the shape the
// unwrapped path produces.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CHECK: structTypeLayout(
// CHECK-LABEL: func %main
