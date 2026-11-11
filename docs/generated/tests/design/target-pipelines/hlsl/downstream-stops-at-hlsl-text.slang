//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#downstream-dxc--fxc
//META: doc_section_digest=d2b70f7c177c7e83d711597b55d20014285aa7ccfee5b7c11ead561d668f0bfa
//META: purpose=`-target hlsl` stops at HLSL text; no DXIL, no SPIR-V, no bytecode is emitted, only HLSL source.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `-target hlsl` terminates at the HLSL text artifact and
// never hands the module to a downstream binary compiler. The test compiles
// an ordinary compute kernel and CHECKs for HLSL-canonical source tokens
// (the pack_matrix pragma, an `RWStructuredBuffer<...>` declaration, and a
// `void main(` definition); their presence proves the output is HLSL source
// text rather than DXIL or D3D bytecode.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #pragma pack_matrix(column_major)
// CHECK: RWStructuredBuffer<int{{.*}}> outBuf_{{[0-9]+}}
// CHECK: void main(
