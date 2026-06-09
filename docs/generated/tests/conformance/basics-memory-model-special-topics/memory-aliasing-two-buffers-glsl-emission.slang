//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#memory-aliasing-via-binding
//META: doc_section_digest=0cb77e4bc46ab9a7f3eefef7b0e375bb9755811b790ee7577f7b13bbf467bf6b
//META: purpose=Two `RWStructuredBuffer<int>` resources emit as independent storage-buffer declarations on GLSL, Metal, CUDA, C++, and WGSL; the compiler assigns them distinct bindings so the client API can alias them at the storage layer.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that two separate `RWStructuredBuffer` resources emit as independently
// bound storage-buffer declarations on every text-emit backend other than
// SPIRV (memory-aliasing-two-buffers-spirv-emission.slang).

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

RWStructuredBuffer<int> bufA;
RWStructuredBuffer<int> bufB;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bufA[0] = 42;
    int val = bufB[0];
    bufA[1] = val;
}

// GLSL: layout(std430, binding = {{[0-9]+}}) buffer StructuredBuffer_int_t_{{[0-9]+}}
// GLSL: layout(std430, binding = {{[0-9]+}}) buffer StructuredBuffer_int_t_{{[0-9]+}}

// MTL: int device{{.*}} bufA_{{[0-9]+}};
// MTL: int device{{.*}} bufB_{{[0-9]+}};

// CUDA: RWStructuredBuffer<int> bufA_{{[0-9]+}};
// CUDA: RWStructuredBuffer<int> bufB_{{[0-9]+}};

// CPP: RWStructuredBuffer<int32_t> bufA_{{[0-9]+}};
// CPP: RWStructuredBuffer<int32_t> bufB_{{[0-9]+}};

// WGSL: var<storage, read_write> bufA_{{[0-9]+}} : array<i32>
// WGSL: var<storage, read_write> bufB_{{[0-9]+}} : array<i32>
