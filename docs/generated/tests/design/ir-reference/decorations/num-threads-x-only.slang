//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=Boundary - numthreads with a single x argument defaults missing y and z dimensions to 1, surfacing as numThreads(x:Int, 1:Int, 1:Int).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc lists `numThreads` as variadic (min=3), but
// the front-end accepts the shorter `[numthreads(x)]` form by
// defaulting y and z to 1. Verify the IR observes all three
// operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(64)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [numThreads(64 : Int, 1 : Int, 1 : Int)]
// CHECK: func %main
