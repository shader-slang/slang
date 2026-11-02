// On GLSL a `T*` is a buffer reference. GLSL has no null literal for one, so a null pointer is
// constructed from address zero, and a block member cannot be `void`, so the buffer-reference
// block of a `void*` holds a `uint` (https://github.com/shader-slang/slang/issues/14082).
//
// `computeMain` compares null and non-null pointers, writes the address of a null `float4*`, whose
// type has no other use, and reads through `int*` pointers cast from a `void*`. Its values are
// checked on Vulkan via GLSL; direct SPIR-V and CUDA are controls.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSLSPV):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -output-using-type

//TEST_INPUT: set q = ubuffer(data=[5 6 7], stride=4);
uniform int* q;
//TEST_INPUT: set v = ubuffer(data=[9], stride=4);
uniform void* v;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

// No block has a `void` member, and the `float4*` block is declared although only a null literal
// has that type.
// GLSL-NOT: void _data
// GLSL-DAG: uint _data;
// GLSL-DAG: vec4 _data;
// GLSL-NOT: void _data
// GLSL: main(
// GLSL: = BufferPointer_{{[a-zA-Z0-9_]+}}(0UL);
// GLSL: [uint(2)] = int(uint64_t(BufferPointer_{{[a-zA-Z0-9_]+}}(0UL)));

// glslang accepts the GLSL, which the Vulkan rows check only on a machine with a GPU.
// GLSLSPV: OpEntryPoint GLCompute

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // `outputBuffer` starts as zeros, so `p` stays null and `r` takes `q`. The branches read
    // runtime data, so the null literals reach the emitter instead of being folded away.
    int* p = nullptr;
    if (outputBuffer[0] != 0)
        p = q;
    int* r = nullptr;
    if (outputBuffer[0] == 0)
        r = q;
    float4* f4 = nullptr;
    void* asVoid = (void*)q;

    outputBuffer[0] = (p != nullptr) ? 1 : 0;
    outputBuffer[1] = (r != nullptr) ? 1 : 0;
    outputBuffer[2] = int(uint64_t(f4));
    outputBuffer[3] = ((int*)asVoid)[2];
    outputBuffer[4] = *(int*)v;
}

// BUF: 0
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 7
// BUF-NEXT: 9
