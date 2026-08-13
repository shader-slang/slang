//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-d-wgsl-emit-and-downstream-tools
//META: doc_section_digest=c09ee21d10b1a5dd61a59485fecdb8390047276650f314609a9a891da39ac9d3
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
