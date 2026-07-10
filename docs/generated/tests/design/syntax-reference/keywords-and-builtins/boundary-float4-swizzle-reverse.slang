//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
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
