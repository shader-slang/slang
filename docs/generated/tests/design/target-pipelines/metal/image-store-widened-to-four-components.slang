//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:38:51+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=93b06c89a6012feb3801573c05960ec24c865923e2f34e1c7f3dca8e8dfa82d2
//META: purpose=An `IRImageStore` value is widened to four components, because Metal texture writes take 4-component values.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RWTexture2D<float>` and `RWTexture2D<float2>` stores are widened to
// `float4` (padding with zeros) before the `.write` call.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWTexture2D<float> t1;
RWTexture2D<float2> t2;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    t1[tid.xy] = 1.0;
    t2.Store(tid.xy, float2(1, 2));
}

// CHECK: float4 [[A:_S[0-9]+]] = float4(1.0f, int(0), int(0), int(0));
// CHECK: t1_{{[0-9]+}}.write([[A]],
// CHECK: float4 [[B:_S[0-9]+]] = float4(1.0f, 2.0f, int(0), int(0));
// CHECK: t2_{{[0-9]+}}.write([[B]],
