// Casts from one pointer type to another, and `bit_cast` to a pointer, compile on GLSL as
// constructor-style buffer-reference casts and read the same values as on SPIR-V and CUDA
// (https://github.com/shader-slang/slang/issues/14078).

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -output-using-type
//TEST:SIMPLE(filecheck=MULTI):-target spirv-asm -emit-spirv-via-glsl -entry multiLevelMain -stage compute

struct S
{
    int a;
    float b;
}

struct Holder
{
    float* p;
}

// 1065353216 and 1073741824 are the bits of 1.0 and 2.0.
//TEST_INPUT: set q = ubuffer(data=[1065353216 7 1073741824 5], stride=4);
uniform int* q;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4);
RWStructuredBuffer<float> outputBuffer;

// GLSL: main(
// GLSL: BufferPointer_{{[a-zA-Z0-9_]+}}(

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float* f = (float*)q;
    S* s = (S*)(q + 1);
    int* back = (int*)f;
    Ptr<int, Access.Read> r = Ptr<int, Access.Read>(q);
    float* g = bit_cast<float*>(uint64_t(q));
    Holder h = bit_cast<Holder>(uint64_t(q));

    outputBuffer[0] = *f;
    outputBuffer[1] = float(s->a);
    outputBuffer[2] = s->b;
    outputBuffer[3] = float(back[3] + r[1]);
    outputBuffer[4] = g[2];
    outputBuffer[5] = *h.p;
}

// BUF: 1.0
// BUF-NEXT: 7.0
// BUF-NEXT: 2.0
// BUF-NEXT: 12.0
// BUF-NEXT: 2.0
// BUF-NEXT: 1.0

// Casts between pointer-to-pointer types, which glslang must accept.
// MULTI: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void multiLevelMain(uniform int** pp)
{
    int** ip = (int**)q;
    S** sp = (S**)pp;
    outputBuffer[0] = float(**ip) + (*sp)->b;
}
