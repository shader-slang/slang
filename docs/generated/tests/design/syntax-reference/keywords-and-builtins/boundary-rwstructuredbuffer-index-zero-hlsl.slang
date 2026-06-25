//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=Boundary: `RWStructuredBuffer<T>` from `hlsl.meta.slang` survives into HLSL emit with an `int(0)` indexing expression at the documented lower-edge index.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `builtin-hlsl-structured-buffer.slang`. That test
// observes the type name in HLSL emit. This one fixes the index at
// the lower-edge boundary (0) and checks that the indexing expression
// also survives to HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    //CHECK: RWStructuredBuffer
    //CHECK: outBuf
    outBuf[0] = 7;
}
