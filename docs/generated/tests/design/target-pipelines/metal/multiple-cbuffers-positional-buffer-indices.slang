//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=2a4fe67f3530d551446e5a52ff530daaceb47c541972fbd77fc4832e5e0aa6a6
//META: purpose=Boundary: two `cbuffer` blocks each receive their own `SLANG_ParameterGroup_<name>_0` wrapper and sequential positional `[[buffer(N)]]` indices — cbuffer slots share the buffer slot space with RW buffers.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: two cbuffers + one RW buffer. The cbuffers wrap
// into `SLANG_ParameterGroup_A_0` / `SLANG_ParameterGroup_B_0` and
// share the buffer slot space with `outBuf` at positions 0, 1, 2.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

cbuffer A { float a; }
cbuffer B { float b; }
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = a + b + float(tid.x);
}

// CHECK: struct SLANG_ParameterGroup_A_0
// CHECK: struct SLANG_ParameterGroup_B_0
// CHECK: buffer(2)
// CHECK-SAME: buffer(0)
// CHECK-SAME: buffer(1)
