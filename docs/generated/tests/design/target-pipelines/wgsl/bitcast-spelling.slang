//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-c-wgsl-legalization-lowering-phi-elimination
//META: doc_section_digest=a15f36e90ce71c175813e4f9aaae1546ac333d5b71b1977e258855334d9377bf
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
