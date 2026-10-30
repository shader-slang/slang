// On GLSL only a user pointer is a buffer reference, so only its null literal is constructed from
// address zero (https://github.com/shader-slang/slang/issues/14082). A `groupshared` pointer has
// no GLSL type to construct, so its null is written as `0`, and comparing it with `nullptr` still
// compiles through glslang.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSLSPV):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4);
RWStructuredBuffer<int> outputBuffer;

// GLSL: main(
// GLSL-NOT: (0UL)
// GLSLSPV: OpEntryPoint GLCompute

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Ptr<int, Access.ReadWrite, AddressSpace.GroupShared> shared = nullptr;
    outputBuffer[0] = (shared == nullptr) ? 1 : 0;
}

// BUF: 1
