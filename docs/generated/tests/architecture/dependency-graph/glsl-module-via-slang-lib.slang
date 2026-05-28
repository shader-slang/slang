//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:59:05+00:00
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/generated/design/architecture/dependency-graph.md#edges-intra-project-only
//META: doc_section_digest=9710717313ceae695df9e18bae5d2ef751a05dfd90122a44daa3fba34798a676
//META: purpose=The `glslModule -> coreLib` edge plus `slangLib -> coreModule` linkage make the GLSL module reachable from a normal slangc invocation; `import glsl;` resolves with no extra search paths.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the slangc-observable consequence of the `glslModule ->
// coreLib` edge combined with the `slangLib -> coreModule` linkage
// (the GLSL module is shipped inside libslang the same way the core
// module is): the module identifier `glsl` resolves with no
// additional `-I` search paths. If the GLSL module library were not
// linked into libslang, the `import glsl;` would fail with an
// unresolved-module diagnostic.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

import glsl;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    //CHECK: void main
}
