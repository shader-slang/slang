//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=355a74cccddb8eb52d0513615291bc6d928b55d9bdd73146a80f6df81b210ae4
//META: purpose=lowerBitCast renders Slang's asuint(float) bitcast as Metal's as_type<uint>(...).
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `lowerBitCast` runs for Metal (`reqSet.bitcast`). The Metal
// emitter renders the resulting IR bitcast as
// `as_type<uint>(...)` (Apple's reinterpret-bits builtin).

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform float f;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = asuint(f) + tid.x;
}

// CHECK: as_type<uint>(
