//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -shaderobj
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

// `__intrinsic_op(0)` is the identity intrinsic, but the declared return type
// of the call is binding: `int get()` reinterprets the bits of the UInt-backed
// struct as int. Issue #14576. This file is the issue's exact shader, extracted
// from strongly-typed-id.slang so the SPIR-V assembly check compiles it in
// isolation; the SPIRV pins below assert the re-typed value is a bit-reinterpret.

__intrinsic_type(UInt)
struct MyId2
{
    __init(uint val) { this = __slang_noop_cast<MyId2>(val); }

    __intrinsic_op(0) int get();
}

int test2(MyId2 id)
{
    if (id.get() == 4)
    {
        return id.get();
    }
    return 0;
}

// The `int get()` call reinterprets the uint bits as int; it must not
// value-convert them.
// SPIRV: OpBitcast %int %{{[a-zA-Z0-9_]+}}
// SPIRV-NOT: OpSConvert

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: 4
    outputBuffer[tid.x] = test2(MyId2(4));
}
