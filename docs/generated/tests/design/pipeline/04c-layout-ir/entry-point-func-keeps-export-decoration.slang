//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-entry-point-steps
//META: doc_section_digest=2051f00317fddb1d324980572e90d6c58961dbf4384bdc38e7d5acf46a5ce786
//META: purpose=The layout-module entry-point stub is wired to the executable-IR definition by mangled name, so after link the entry point carries a matching export decoration.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the stub the layout module materialises for the entry
// point is linked to the real definition by mangled name rather than
// duplicating it. CHECK asserts the post-link `func %main` still carries an
// `[export("...main...")]` linkage decoration next to its `[layout(...)]`,
// i.e. the stub resolved against the executable-side symbol.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CHECK: [export("{{.*}}main{{.*}}")]
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: func %main
