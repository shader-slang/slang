//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint-entrypointdecoration
//META: doc_section_digest=d3d96830534992c13d27b2f3fc509ca407eb67250f8a01585d992e27d7abbae9
//META: purpose=A function marked with shader/numthreads attributes lowers to a func with an entryPoint decoration carrying profile-int, entry-name, and module-name operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc: "entryPoint marks a function as a shader entry
// point. Its three operands are the profile (an IRIntLit tag), the
// user-visible name, and an optional module name."

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 4, 2)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK: func %main
