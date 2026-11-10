// shader-slang/slang#14030: groupshared arrays sized by a specialization constant on GLSL and WGSL.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// GLSL: layout(constant_id = 0)
// GLSL-NEXT: const int kTile_0 = 4;
// GLSL: const int [[WIDE:_S[0-9]+]] = 1 + 2 * kTile_0;
// GLSL-DAG: shared float {{ *}}tile_0[kTile_0];
// GLSL-DAG: shared float {{ *}}wide_0{{\[}}[[WIDE]]{{\]}};

// WGSL: @id(0) override kTile_0 : i32 = i32(4);
// WGSL-DAG: var<workgroup> tile_0 : array<f32, kTile_0>;
// WGSL-DAG: var<workgroup> wide_0 : array<f32, i32(1) + i32(2) * kTile_0>;

[SpecializationConstant]
const int kTile = 4;

groupshared float tile[kTile];
groupshared float wide[kTile * 2 + 1];

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint gi: SV_GroupIndex)
{
    tile[gi] = gi * 10;
    wide[gi * 2] = gi;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[gi] = tile[(gi + 1) % kTile] + wide[gi * 2];
}

// BUF: 10
// BUF-NEXT: 21
// BUF-NEXT: 32
// BUF-NEXT: 3
