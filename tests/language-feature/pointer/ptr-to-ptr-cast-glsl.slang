// On GLSL a `T*` is a buffer reference, and a cast that makes one reaches the emitter as a
// `bitCast` to a pointer, which is written as a buffer-reference constructor
// (https://github.com/shader-slang/slang/issues/14078). The existential case is in
// ptr-field-existential-glsl.slang.
//
// `computeMain` reads one buffer through a cast to another pointer type, to a struct pointer, back
// again, to a `Read` pointer, through `bit_cast` to a pointer and to a struct that holds one, and
// through an `int64_t`.
// Its values are checked on Vulkan via GLSL; direct SPIR-V and CUDA are controls.
//
// `multiLevelMain` makes pointers to pointers. Dereferencing `(int**)q` reads the input data as an
// address, so it is only compiled, through glslang.
//
// `narrowIntMain` converts between a `T*` and a 32-bit `uint`, which GLSL spells through a
// `uint64_t`. No compiler pass makes such a conversion today, so the test declares it with
// `__intrinsic_op`.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly -Xslang... -DNO_SIGNED_PTR_TO_INT -X.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -output-using-type
//TEST:SIMPLE(filecheck=MULTI):-target spirv-asm -emit-spirv-via-glsl -entry multiLevelMain -stage compute
//TEST:SIMPLE(filecheck=NARROW_GLSL):-target glsl -entry narrowIntMain -stage compute
//TEST:SIMPLE(filecheck=NARROW):-target spirv-asm -emit-spirv-via-glsl -entry narrowIntMain -stage compute

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
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0], stride=4);
RWStructuredBuffer<float> outputBuffer;

// GLSL: main(
// GLSL: [uint(0)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(1)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(2)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(3)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(4)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(5)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(
// GLSL: [uint(6)] = {{.*}}BufferPointer_{{[a-zA-Z0-9_]+}}(uint64_t(

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
#if !defined(NO_SIGNED_PTR_TO_INT)
    int* viaSigned = Ptr<int>(int64_t(q));
#else
    // Direct SPIR-V signed ptr->int is #14087; covered by its own test.
    int* viaSigned = q;
#endif

    outputBuffer[0] = *f;
    outputBuffer[1] = float(s->a);
    outputBuffer[2] = s->b;
    outputBuffer[3] = float(back[3] + r[1]);
    outputBuffer[4] = g[2];
    outputBuffer[5] = *h.p;
    outputBuffer[6] = float(viaSigned[1]);
}

// BUF: 1.0
// BUF-NEXT: 7.0
// BUF-NEXT: 2.0
// BUF-NEXT: 12.0
// BUF-NEXT: 2.0
// BUF-NEXT: 1.0
// BUF-NEXT: 7.0

// MULTI: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void multiLevelMain(uniform int** pp)
{
    int** ip = (int**)q;
    S** sp = (S**)pp;
    outputBuffer[0] = float(**ip) + (*sp)->b;
}

__intrinsic_op(CastIntToPtr)
int* castUIntToPtr(uint value);

__intrinsic_op(CastPtrToInt)
uint castPtrToUInt(int* pointer);

// NARROW_GLSL: BufferPointer_{{[a-zA-Z0-9_]+}}(uint64_t({{.*}}lo_0))
// NARROW_GLSL: uint(uint64_t({{.*}}q_0))
// NARROW: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void narrowIntMain(uniform uint lo)
{
    outputBuffer[0] = float(*castUIntToPtr(lo)) + float(castPtrToUInt(q));
}
