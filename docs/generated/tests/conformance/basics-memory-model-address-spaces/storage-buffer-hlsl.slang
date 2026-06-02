//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`StructuredBuffer<T>` (read-only) belongs to Storage buffer address space; HLSL emits it bound to a `t` register (SRV), while `RWStructuredBuffer<T>` binds to a `u` register (UAV).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the HLSL register-binding distinction between read-only and read-write Storage
// buffer variants. `StructuredBuffer<T>` (read-only) and `RWStructuredBuffer<T>` (read-write)
// both occupy the Storage buffer address space; CHECK asserts the emitted HLSL binds the
// read-only form to a `t` register (SRV) and the read-write form to a `u` register (UAV).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

StructuredBuffer<float>   robuf;
RWStructuredBuffer<float> rwbuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    float v = robuf[0];
    rwbuf[0] = v + 1.0;
    output[0] = rwbuf[0];
}

// Read-only StructuredBuffer -> t register (shader resource view).
// CHECK: StructuredBuffer<float{{.*}}> {{.*}} : register(t
// Read-write RWStructuredBuffer -> u register (unordered access view).
// CHECK-DAG: RWStructuredBuffer<float{{.*}}> {{.*}} : register(u
