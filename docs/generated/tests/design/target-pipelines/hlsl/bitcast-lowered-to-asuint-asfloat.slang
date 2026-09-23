//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#requiredloweringpassset-flags
//META: doc_section_digest=c3103ee2a38af2bf635bdd9987e457e5b1fbf9bc975f679755d345340e97d6dc
//META: purpose=A bit-cast in the module sets the `bitcast` gate that selects `lowerBitCast`, whose HLSL result is the reinterpreting intrinsic pair — `asuint(...)` for float-to-uint and `asfloat(...)` for uint-to-float — rather than a value-converting cast.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the emit-visible consequence of the `bitcast` gate row.
// Both directions are exercised in one kernel so the CHECKs can
// distinguish a bit-reinterpretation from a numeric conversion:
// a numeric cast of the same operands would emit `uint(...)` /
// `float(...)` constructor-style spellings, which the bundle
// already pins elsewhere. Operands are read from a buffer and from
// the dispatch id so nothing is constant-folded before the gate is
// computed.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<float> inBuf;
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = asuint(inBuf[tid.x]);
    outBuf[tid.x + 1] = asuint(bit_cast<float>(tid.y));
}

// CHECK-DAG: = asuint(inBuf_{{[0-9]+}}.Load(
// CHECK-DAG: = asuint(asfloat(tid_{{[0-9]+}}.y));
