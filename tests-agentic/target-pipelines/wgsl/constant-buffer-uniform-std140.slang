//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=287321dc153823860396c030325e4ad4c0f6064005fa37b19c8ba53087dcd3e6
//META: purpose=ConstantBuffer<T> emits as a std140-shaped struct with @align field annotations bound as var<uniform>
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A ConstantBuffer<S> uses WGSL's uniform address space; the
// struct fields carry explicit @align(N) annotations matching the
// std140 layout.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

struct CB { float a; float b; float c; };
ConstantBuffer<CB> cb;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = cb.a + cb.b + cb.c;
}

// CHECK: struct CB_std140
// CHECK: @align(
// CHECK: var<uniform>
// CHECK-SAME: cb_{{[0-9]+}}
