//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:59:05+00:00
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/generated/design/architecture/dependency-graph.md#notable-invariants
//META: doc_section_digest=f07179a73dae3e0e42bafd2eeae675c59d495befe1b03e0570069dff2900a4f2
//META: purpose=In the default build the core module is linked into `slang` (SLANG_EMBED_CORE_MODULE); built-in vector types resolve without any `import`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the slangc-observable consequence of the
// `coreModule -> coreLib` and `slangLib -> coreModule` edges in the
// dependency graph, combined with the "core module is linked
// optionally" invariant: when the default build's
// SLANG_EMBED_CORE_MODULE selector chooses the embedded flavor, the
// built-in vector types and constructors supplied by the core module
// are visible to user code with no import statement.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int4 v = int4(2, 3, 5, 7);
    float3 f = float3(0.5, 1.5, 2.0);
    //CHECK: 17
    printf("%d\n", v.x + v.y + v.z + v.w);
    //CHECK: 4
    printf("%d\n", int(f.x + f.y + f.z));
}
