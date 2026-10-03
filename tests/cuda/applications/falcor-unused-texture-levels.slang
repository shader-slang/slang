// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -compute-dispatch 5,1,1 -capability cuda_sm_8_0 -Xslang -ignore-capabilities -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -compute-dispatch 5,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -ignore-capabilities -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -compute-dispatch 5,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -ignore-capabilities -Xslang -O3
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -ignore-capabilities -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -ignore-capabilities -capability cuda_sm_8_0 -O3 -o -

//DIAGNOSTIC_TEST:SIMPLE(diag=LIVE): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -ignore-capabilities -DREAD_OLD=1 -capability cuda_sm_8_0 -O0 -o falcor-live-texture-levels.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=LIVE): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -ignore-capabilities -DREAD_OLD=1 -capability cuda_sm_8_0 -O3 -o falcor-live-texture-levels.ptx

// Suppress the public source-level stage requirement to exercise the same typed IR that
// reaches preflight from Falcor's linked module. Live count queries remain rejected by NVVM.
// EnvMapLight.power_estimate overwrites the mip count on CUDA. The unused query must disappear
// through normal DCE; its live compute-stage form remains unsupported. The extra unused base/
// layer queries check the same immutable metadata family without changing texel read semantics.
//TEST_INPUT: Texture2D(size=16, content=one, mipMaps=5):name texture
Texture2D<float4> texture;
//TEST_INPUT: Texture2D(size=16, content=one, arrayLength=3, mipMaps=5):name arrayTexture
Texture2DArray<float4> arrayTexture;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint w, h, levels, layers;
    arrayTexture.GetDimensions(w, h, levels);
    arrayTexture.GetDimensions(tid.x, w, h, layers, levels);
    uint3 dims;
    texture.GetDimensions(tid.x, dims.x, dims.y, dims.z);
#ifdef READ_OLD
    output[5] = dims.z;
#endif
    dims.z = firstbithigh(dims.x) + 1;
    output[tid.x] = uint(dims.x == (16 >> tid.x) && dims.y == (16 >> tid.x) && dims.z == 5-tid.x);
}
// PTX-NOT: txq.num_mipmap_levels
// PTX-NOT: txq.array_size
// PTX: .visible .entry
// PTX-NOT: txq.num_mipmap_levels
// PTX-NOT: txq.array_size
// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}

//LIVE: direct NVVM lowering does not support Slang IR instruction or shape 'texture mip count stage'
