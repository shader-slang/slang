//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=`uniform` scalar/struct globals are packed into a GlobalParams struct that is passed as a `constant*` parameter via introduceExplicitGlobalContext (Metal fallthrough chain).
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `introduceExplicitGlobalContext` (via the Metal fallthrough
// into the ShaderLLVMIR arm) collects loose `uniform` globals
// into a `GlobalParams_0` struct and passes a `constant*` of
// that struct as a kernel parameter.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

uniform int scale;
uniform float bias;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(tid.x) * float(scale) + bias;
}

// CHECK: struct GlobalParams_{{[0-9]+}}
// CHECK: GlobalParams_{{[0-9]+}} constant* globalParams_{{[0-9]+}}
