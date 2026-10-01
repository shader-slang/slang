// A write to a component of a texel at a dynamic index (`tex[i][k] = v`) cannot be lowered to
// a swizzle, so the read-modify-write selects each component of the new texel on the index
// (shader-slang/slang#12627).

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -shaderobj
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

//TEST_INPUT: RWTexture2D(format=RGBA32Float, size=4, content = zero, mipMaps = 1):name tex
RWTexture2D<float4> tex;

//TEST_INPUT: ubuffer(data=[2 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint2 i = uint2(1, 2);
    // The index comes from the input buffer so that it stays dynamic; it is echoed back as the
    // first output value.
    uint k = outputBuffer[0];

    tex[i] = float4(1.0f, 2.0f, 3.0f, 4.0f);
    tex[i][k] = 7.0f;

    float4 v = tex[i];
    outputBuffer[1] = uint(v.x);
    outputBuffer[2] = uint(v.y);
    outputBuffer[3] = uint(v.z);
    outputBuffer[4] = uint(v.w);
}

// BUF: 2
// BUF-NEXT: 1
// BUF-NEXT: 2
// BUF-NEXT: 7
// BUF-NEXT: 4

// METAL: [[T:_S[0-9]+]] = {{.*}}.read(
// METAL: .write(float4(select([[T]][int(0)], 7.0f, {{.*}} == 0U), select([[T]][int(1)], 7.0f, {{.*}} == 1U), select([[T]][int(2)], 7.0f, {{.*}} == 2U), select([[T]][int(3)], 7.0f, {{.*}} == 3U)),

// GLSL: vec4 [[T:_S[0-9]+]] = imageLoad(
// GLSL: imageStore({{.*}}vec4({{.*}} == 0U ? 7.0 : [[T]][0], {{.*}} == 1U ? 7.0 : [[T]][1], {{.*}} == 2U ? 7.0 : [[T]][2], {{.*}} == 3U ? 7.0 : [[T]][3]))

// SPIRV: OpImageRead
// SPIRV: OpSelect
// SPIRV: OpImageWrite

// CUDA: float4 [[T:_S[0-9]+]] = surf2Dread<float4 >(
// CUDA: surf2Dwrite<float4 >(make_float4 (_slang_select({{.*}} == 0U, 7.0f,[[T]].x), _slang_select({{.*}} == 1U, 7.0f,[[T]].y), _slang_select({{.*}} == 2U, 7.0f,[[T]].z), _slang_select({{.*}} == 3U, 7.0f,[[T]].w)),

// PTX: .entry
// PTX: suld
// PTX: sust
