//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=Boundary: `float4` is the maximum-width vector documented in `core.meta.slang`; the four-component `wzyx` swizzle is the full-length reverse, the largest swizzle on a `float4`.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the full-length swizzle on the widest documented vector
// width. A parser that lost any component of the swizzle would shift
// the printed sequence.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 v = float4(1.0, 2.0, 3.0, 4.0);
    float4 r = v.wzyx;
    // CHECK: r=4 3 2 1
    printf("r=%d %d %d %d\n", int(r.x), int(r.y), int(r.z), int(r.w));
}
