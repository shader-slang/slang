//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#storage-type-legalization-casts
//META: doc_section_digest=221b34de87a25c557e22e8683a229aec25e86798194a0bdcacb5ad27c36b69b1
//META: purpose=A call to the core-module `asDynamicUniform<T>` emits a `TreatAsDynamicUniform` inst wrapping the marked value, and the uniformity pass consumes it before emit.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both the presence of the marker inst and the fact that it is a
// compile-time annotation with no target-code counterpart. The marked value is
// a thread id so nothing folds it away; the IR directive pins
// `TreatAsDynamicUniform(<value>)` in the dump, and the HLSL directive (run
// with the uniformity checker enabled, which is what reads the marker) shows
// the emitted code assigning the unwrapped thread id with no trace of the
// marker.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main -validate-uniformity
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main -validate-uniformity

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint u = asDynamicUniform(tid.x);
    outputBuffer[0] = (int)u;
}

// IR: = TreatAsDynamicUniform(%{{[A-Za-z0-9_]+}})

//HLSL-NOT: TreatAsDynamicUniform
//HLSL: outputBuffer{{[A-Za-z0-9_]*}}[int(0)] = int(tid{{[A-Za-z0-9_]*}}.x)
