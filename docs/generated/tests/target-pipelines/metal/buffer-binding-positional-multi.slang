//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=With multiple StructuredBuffer params Metal emits sequential positional [[buffer(N)]] indices (0, 1, 2 ...), not driven by HLSL register() or vk::binding().
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal [[buffer(N)]] indices are assigned positionally by the
// emitter. With three buffer parameters we expect 0, 1, 2 in
// declaration order; no register() / vk::binding() drives the
// numbering on Metal.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

StructuredBuffer<int> a;
StructuredBuffer<int> b;
RWStructuredBuffer<int> c;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    c[tid.x] = a[tid.x] + b[tid.x];
}

// Bindings are positional in declaration order: a→0, b→1, c→2.
// The output emits c (the RW buffer) first in the parameter list,
// so the marker order in the kernel signature is 2, 0, 1.
// CHECK: kernel{{.*}} void main_0
// CHECK-SAME: buffer(2)
// CHECK-SAME: buffer(0)
// CHECK-SAME: buffer(1)
