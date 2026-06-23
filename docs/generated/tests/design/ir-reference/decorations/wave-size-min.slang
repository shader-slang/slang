//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=Boundary - WaveSize(4) at the documented small-lane edge produces waveSize(4:Int) in the dump.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - `[WaveSize(N)]` requires a power-of-two lane count.
// 4 is the smallest accepted value (E31103 fires on non-power-of-
// two such as 7); pin the resulting waveSize decoration integer.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
[WaveSize(4)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [waveSize(4 : Int)]
// CHECK: func %main
