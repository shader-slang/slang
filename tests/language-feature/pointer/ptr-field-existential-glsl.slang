// A `T*` field of an existential value is packed into, and unpacked from, a `uvec2`, which GLSL
// converts with a buffer-reference constructor under GL_EXT_buffer_reference_uvec2
// (https://github.com/shader-slang/slang/issues/14078).
//
// render-test always dispatches `computeMain`, so each variant is a `-D` define rather than a
// separate entry point. `WITH_OTHER_CAST` adds an unrelated pointer cast; it makes
// `lowerBitCast` run, which rewrites the `uvec2` packing into `uint64_t` leaves, so the
// existential reaches the emitter in the other form and must read the same value.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -output-using-type -emit-spirv-via-glsl -Xslang... -DWITH_OTHER_CAST -X.
//TEST:SIMPLE(filecheck=PACK_ONLY):-target spirv-asm -emit-spirv-via-glsl -entry packOnlyMain -stage compute

interface IGetter
{
    float get();
}

struct PtrGetter : IGetter
{
    int* p;
    float get() { return float(p[1]); }
}

struct ValueGetter : IGetter
{
    float v;
    float get() { return v; }
}

// 1065353216 is the bits of 1.0.
//TEST_INPUT: set q = ubuffer(data=[1065353216 7 1073741824 5], stride=4);
uniform int* q;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4);
RWStructuredBuffer<float> outputBuffer;

// The `if` depends on buffer data, so `getter` stays an existential instead of being
// specialized to `PtrGetter`.
// GLSL: #extension GL_EXT_buffer_reference_uvec2 : require
// GLSL: = BufferPointer_{{[a-zA-Z0-9_]+}}(uvec2(
// BUF: 7.0
// BUF-NEXT: 1.0

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    IGetter getter;
    if (q[3] == 5)
    {
        PtrGetter a;
        a.p = q;
        getter = a;
    }
    else
    {
        ValueGetter b;
        b.v = 3.0;
        getter = b;
    }
    outputBuffer[0] = getter.get();
#if defined(WITH_OTHER_CAST)
    outputBuffer[1] = *(float*)q;
#else
    outputBuffer[1] = q[0] == 1065353216 ? 1.0 : 0.0;
#endif
}

// Storing an existential packs its `T*` field and never unpacks it, so only the pack side of the
// conversion asks for GL_EXT_buffer_reference_uvec2; glslang rejects the GLSL without it.
// PACK_ONLY: OpEntryPoint

[shader("compute")]
[numthreads(1, 1, 1)]
void packOnlyMain(uniform RWStructuredBuffer<IGetter> getterOut)
{
    PtrGetter a;
    a.p = q;
    getterOut[0] = a;
}
