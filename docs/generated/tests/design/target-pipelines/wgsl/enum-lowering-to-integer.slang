//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-a-link-and-entry-point-prep
//META: doc_section_digest=86d41dbf4010f25a2b2aa029b4741aa43bbc09056a8dc33efa096e2b729a4645
//META: purpose=lowerEnumType replaces enumerator references with the underlying integer literal in WGSL emit
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The enum's underlying integer value is the only thing that
// survives to emit. The enumerator name 'Green' should not appear
// in the output.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

enum Color { Red = 0, Green = 1, Blue = 2 };

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(Color.Green);
}

// CHECK: outBuf_{{[0-9]+}}{{.*}}i32(1)
