//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-entry-point-steps
//META: doc_section_digest=2051f00317fddb1d324980572e90d6c58961dbf4384bdc38e7d5acf46a5ce786
//META: purpose=The final per-entry-point step attaches an IRLayoutDecoration to the stub entry-point function, observable as a [layout(%N)] on func %main.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the entry-point stub built by the layout module carries a
// layout decoration - the decoration reflection and the linker query.
// CHECK asserts a `[layout(%N)]` decoration immediately precedes
// `func %main` in the first post-link snapshot; the pre-link executable IR
// has no such decoration, so it can only come from the layout module.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: func %main
