//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// The timing harness must restore writable array layers and full 3D depth outside the events.
// Multiple mip levels and cube faces are also present in the snapshot inventory.
// Use NVRTC for this harness contract: direct NVVM does not yet lower layered surface writes.
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content=zero, arrayLength=2, mipMaps=2):name arrayTexture
RWTexture2DArray<uint> arrayTexture;
//TEST_INPUT: RWTexture3D(format=R32Uint, size=4, content=zero, mipMaps=2):name volumeTexture
RWTexture3D<uint> volumeTexture;
//TEST_INPUT: TextureCube(format=R32Float, size=4, content=one, mipMaps=2):name cubeTexture
TextureCube<float> cubeTexture;
//TEST_INPUT: Sampler(filteringMode=point):name pointSampler
SamplerState pointSampler;
//TEST_INPUT: set output = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    arrayTexture[uint3(1, 1, 1)] += 3;
    volumeTexture[uint3(1, 1, 3)] += 5;
    output[0] = arrayTexture[uint3(1, 1, 1)];
    output[1] = volumeTexture[uint3(1, 1, 3)];
    output[2] = uint(cubeTexture.SampleLevel(pointSampler, float3(-1, 0, 0), 1));
}

// CHECK: type: uint32_t
// CHECK-NEXT: 3
// CHECK-NEXT: 5
// CHECK-NEXT: 1
