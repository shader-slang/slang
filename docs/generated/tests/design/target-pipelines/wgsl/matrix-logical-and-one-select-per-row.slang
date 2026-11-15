//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizelogicalandor
//META: doc_section_digest=0f95d85b7e4917c52c5df24093e28b4af12cd02e40737f7e612285fa2f66c08c
//META: purpose=A bool2x2 logical And is rewritten to one vector select per matrix row (legalizeMatrixTypes), not a matrix operation
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeLogicalAndOr leaves the matrix And alone; legalizeMatrixTypes then
// splits it into one vector And per row, each emitted as select(...).

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> inBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool2x2 a = bool2x2(inBuf[0] > 0.0f, inBuf[1] > 0.0f);
    bool2x2 b = bool2x2(inBuf[2] > 0.0f, inBuf[3] > 0.0f);
    bool2x2 c = a && b;
    outBuf[0] = c[0][0] ? 1.0f : 0.0f;
    outBuf[1] = c[1][1] ? 1.0f : 0.0f;
}

// CHECK: select(vec2<bool>(false),
// CHECK: select(vec2<bool>(false),
