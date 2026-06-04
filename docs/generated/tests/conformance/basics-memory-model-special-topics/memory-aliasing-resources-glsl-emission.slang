//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#memory-aliasing-via-binding
//META: doc_section_digest=0cb77e4bc46ab9a7f3eefef7b0e375bb9755811b790ee7577f7b13bbf467bf6b
//META: purpose=`ConstantBuffer<T>` and `RWStructuredBuffer<T>` emit as distinct, independently-bound resource declarations on GLSL, Metal, CUDA, C++, and WGSL; neither backend merges them.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `ConstantBuffer<T>` and `RWStructuredBuffer<T>` emit as
// independent resource declarations on every text-emit backend other than
// HLSL (memory-aliasing-resources-hlsl-emission.slang) and SPIRV
// (memory-aliasing-resources-spirv-emission.slang).

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

ConstantBuffer<int> cbuf;
RWStructuredBuffer<int> rwbuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int val = cbuf;
    rwbuf[0] = val;
}

// GLSL: layout({{.*}}) uniform block_{{.*}}
// GLSL: layout(std430, binding = {{[0-9]+}}) buffer StructuredBuffer_int_t_{{[0-9]+}}

// MTL: int constant{{.*}} cbuf_{{[0-9]+}};
// MTL: int device{{.*}} rwbuf_{{[0-9]+}};

// CUDA: int{{.*}} cbuf_{{[0-9]+}};
// CUDA: RWStructuredBuffer<int> rwbuf_{{[0-9]+}};

// CPP: int32_t{{.*}} cbuf_{{[0-9]+}};
// CPP: RWStructuredBuffer<int32_t> rwbuf_{{[0-9]+}};

// WGSL: var<uniform> cbuf_{{[0-9]+}} : i32
// WGSL: var<storage, read_write> rwbuf_{{[0-9]+}} : array<i32>
