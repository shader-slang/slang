//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint-entrypointdecoration
//META: doc_section_digest=d3d96830534992c13d27b2f3fc509ca407eb67250f8a01585d992e27d7abbae9
//META: purpose=A second user-named helper called from main coexists with the entry-point: both functions surface their own decorations independently in the dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc's notable-opcodes section: "The link-time pass walks
// every entryPoint decoration to select the functions exposed in
// the final binary." A non-entry helper called from main does not
// carry entryPoint, but main does — confirming the decoration is
// scoped to the entry-point function only.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int helperOnly(int x) { return x + 17; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = helperOnly(int(tid.x));
}

// CHECK-DAG: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK-DAG: [nameHint("helperOnly")]
