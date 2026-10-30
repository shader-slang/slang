// On GLSL a `T*` is a buffer reference, and a cast that makes one reaches the emitter as a
// `bitCast` to a pointer, which is written as a buffer-reference constructor
// (https://github.com/shader-slang/slang/issues/14078). The existential case is in
// ptr-field-existential-glsl.slang.
//
// `computeMain` reads one buffer through a cast to another pointer type, to a struct pointer, back
// again, to a `Read` pointer, and through `bit_cast` to a pointer and to a struct that holds one.
// Its values are checked on Vulkan via GLSL; direct SPIR-V and CUDA are controls.
//
// `multiLevelMain` makes pointers to pointers. Dereferencing `(int**)q` reads the input data as an
// address, so it is only compiled, through glslang.

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

// Each store reads through a buffer-reference constructor, not a C-style cast.
// GLSL: main(
// GLSL: [uint(0)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(1)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(2)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(3)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(4)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(5)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(

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

// MULTI: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void multiLevelMain(uniform int** pp)
{
    int** ip = (int**)q;
    S** sp = (S**)pp;
    outputBuffer[0] = float(**ip) + (*sp)->b;
}
