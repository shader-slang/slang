//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -shaderobj
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

// `__intrinsic_op(0)` is the identity intrinsic, but the declared return type
// of the call is binding: `int get()` reinterprets the bits of the UInt-backed
// struct as int. Issue #14576. This file holds the issue's declaration set in
// a minimal entry point so the SPIR-V assembly check compiles it in isolation.

__intrinsic_type(UInt)
struct MyId2
{
    __init(uint val) { this = __slang_noop_cast<MyId2>(val); }

    __intrinsic_op(0) int get();
}

// The same re-typing across value kinds: the int bits of 1.5f (0x3FC00000) are
// reinterpreted as float, not value-converted to 1069547520.0.
__intrinsic_type(Int)
struct AsFloat
{
    __init(int val) { this = __slang_noop_cast<AsFloat>(val); }

    __intrinsic_op(0) float get();
}

int test2(MyId2 id)
{
    if (id.get() == 4)
    {
        return id.get();
    }
    return 0;
}

// The re-typed getter calls must emit bitcasts to their declared types. (For a
// same-width pair, SPIR-V spells a numeric reinterpret `OpBitcast` as well;
// `OpSConvert`/`OpUConvert` only appear on width changes this module never
// performs, so the runtime CHECK below is what distinguishes a reinterpret
// from a value conversion.) The float bitcast appears before the int one in
// the emitted function order.
// SPIRV: %{{[a-zA-Z0-9_]+}} = OpBitcast %float %{{[a-zA-Z0-9_]+}}
// SPIRV: %{{[a-zA-Z0-9_]+}} = OpBitcast %int %id
// SPIRV-NOT: OpSConvert
// SPIRV-NOT: OpUConvert

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: 4
    outputBuffer[tid.x] = uint(test2(MyId2(4)));
    // The bit pattern of 1.5f, reinterpreted as float and scaled — 3 when the
    // bits are reinterpreted, 2139095040 when they are value-converted.
    // CHECK: 3
    outputBuffer[4 + tid.x] = uint(AsFloat(1069547520).get() * 2.0f);
    // The same bits reinterpreted directly through the generic identity cast.
    // CHECK: 3
    outputBuffer[8 + tid.x] = uint(__slang_noop_cast<float>(1069547520) * 2.0f);
}
