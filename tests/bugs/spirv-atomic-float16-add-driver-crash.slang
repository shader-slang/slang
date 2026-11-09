// This is a reduction of an NVIDIA Vulkan pipeline-creation crash. The SPIR-V is accepted by
// spirv-val, but the driver faults while compiling the compute pipeline when the device does not
// support VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicAdd.
//
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain -stage compute -emit-spirv-directly -O2
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -emit-spirv-directly -Xslang -O2

// SPIRV: OpCapability AtomicFloat16AddEXT
// SPIRV: OpExtension "SPV_EXT_shader_atomic_float16_add"
// SPIRV: OpAtomicFAddEXT

// TEST_INPUT: ubuffer(data=[0.h], stride=2):out, name=gradientBuffer
RWStructuredBuffer<half> gradientBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    __atomic_reduce_add(gradientBuffer[0], half(1.0));

    // BUFFER: 1
}
