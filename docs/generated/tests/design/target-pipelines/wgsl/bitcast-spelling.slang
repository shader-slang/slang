//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=1a006f5533aa420dc23fbb0ad582fabeafd0bcf79da9890e8a7b8254871f2401
//META: purpose=asuint() / asfloat() reinterprets emit as WGSL bitcast<T>(...) expressions
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// lowerBitCast feeds the WGSL emitter; reinterpret casts (asuint /
// asfloat) become the WGSL bitcast<T>(...) spelling.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = asuint(1.5f);
}

// CHECK: bitcast<u32>
