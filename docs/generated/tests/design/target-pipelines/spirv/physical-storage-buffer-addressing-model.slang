//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=The addressing model in the emitted `OpMemoryModel` is `Logical` unless `requirePhysicalStorageAddressing` moved it, which a buffer-pointer parameter does.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one documented escape from the default addressing model. The
// front matter emits OpMemoryModel with whatever addressing model the module
// ended up in, so a shader that dereferences a uniform pointer -- which needs
// real device addresses -- must shift the whole module to the 64-bit physical
// storage-buffer model. The CHECKs pin that model plus the capability that
// accompanies it, in contrast to the Logical model the sibling tests assert.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct Data { float4 v; };
uniform Data* dataPtr;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = dataPtr[tid.x].v;
}

// CHECK-DAG: OpCapability PhysicalStorageBufferAddresses
// CHECK-DAG: OpExtension "SPV_KHR_physical_storage_buffer"
// CHECK: OpMemoryModel PhysicalStorageBuffer64 GLSL450
