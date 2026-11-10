// A list initializer for an array sized by a specialization constant is not an error when
// optimization removes the array, as it does here where a store is forwarded to the only load
// (#14030).

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-via-glsl

// SPIRV-NOT: E55223
// SPIRV: OpEntryPoint
// GLSL-NOT: E55223
// GLSL: void main()

[SpecializationConstant]
const int kTile = 4;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float sums[kTile] = {};
    sums[0] = 3.0;
    outputBuffer[0] = sums[0];
}

// BUF: 3.0
