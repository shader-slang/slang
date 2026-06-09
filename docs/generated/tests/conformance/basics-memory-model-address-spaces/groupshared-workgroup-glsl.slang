//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`static groupshared` at global scope belongs to the Group-shared address space; GLSL emits it with the `shared` storage qualifier.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `static groupshared` at global scope maps to the Group-shared address space
// on GLSL. CHECK asserts the emitted GLSL uses the `shared` storage qualifier on the array,
// confirming the thread-group-visible memory mapping.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

static groupshared int sharedCounter[16];

[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<int> output)
{
    sharedCounter[tid.x] = int(tid.x) * 2;
    GroupMemoryBarrierWithGroupSync();
    output[tid.x] = sharedCounter[15u - tid.x];
}

// CHECK: shared {{.*}} sharedCounter{{.*}}
// Metal maps groupshared to threadgroup address space.
// METAL: threadgroup {{.*}} sharedCounter{{.*}}
// WGSL maps groupshared to var<workgroup>.
// WGSL: var<workgroup> sharedCounter{{.*}}
