//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -shaderobj
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

// This test routes the re-typed getter through an interface, so the call survives
// lowering as a real IRCall and the mandatory-early inlining pass resolves it —
// the second producer site of the identity intrinsic covered by issue #14576.

interface IGetter
{
    int get();
}

__intrinsic_type(UInt)
struct MyId2
{
    __init(uint val) { this = __slang_noop_cast<MyId2>(val); }

    __intrinsic_op(0) int get();
}

extension MyId2 : IGetter {}

int viaInterface<T:IGetter>(T id)
{
    return id.get();
}

// The re-typed getter call must emit an int-typed bitcast of the uint bits.
// (For a same-width pair, SPIR-V spells a numeric reinterpret `OpBitcast` as
// well; the runtime CHECK below is what distinguishes a reinterpret from a
// value conversion.)
// SPIRV: OpBitcast %int %{{[a-zA-Z0-9_]+}}
// SPIRV-NOT: OpSConvert

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: 4
    outputBuffer[tid.x] = viaInterface(MyId2(4));
}
