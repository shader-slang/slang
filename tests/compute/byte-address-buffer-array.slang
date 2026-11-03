// byte-address-buffer-array.slang
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -d3d12 -profile cs_6_0 -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -cuda -profile cs_6_0 -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -output-using-type

//TEST:SIMPLE(filecheck=CHECK2):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK3):-target spirv -entry computeMain -stage compute -O1
//TEST:SIMPLE(filecheck=CHECK3):-target spirv -emit-spirv-directly -entry computeMain -stage compute -O1

// Confirm compilation of `(RW)ByteAddressBuffer` with aligned load / stores to wider data types.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=buffer
RWByteAddressBuffer buffer;
struct Block {
    float4 val[2];
};
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK-NOT: warning

    // A load with one use is written inside that use. The second row is read before the store
    // at 16 overwrites bytes 20-31.
    // CHECK2: buffer_0.Store(0U,(buffer_0).Load<float4 [int(2)] >(0U));
    // CHECK2: float4 [[ROW1:[A-Za-z0-9_]+]] = float4((buffer_0).Load<float >(20U), (buffer_0).Load<float >(24U), (buffer_0).Load<float >(28U), (buffer_0).Load<float >(32U));
    // CHECK2: buffer_0.Store(16U,float4((buffer_0).Load<float >(4U), (buffer_0).Load<float >(8U), (buffer_0).Load<float >(12U), (buffer_0).Load<float >(16U)));
    // CHECK2: buffer_0.Store(32U,[[ROW1]]);

    // CHECK3: OpAccessChain
    // CHECK3: OpLoad %v4float
    // CHECK3: OpStore
    buffer.Store(0, buffer.LoadAligned<Block>(0));
    // Load at scalar (4-byte) alignment -> per-component loads; store at 16-byte alignment ->
    // the two `float4` rows are stored wide. (Per issue #11545 the alignment must be a power of
    // two and the constant location a multiple of it, so the previous `LoadAligned<Block>(4, 16)`
    // / `StoreAligned(16, ...)` self-contradictory calls are expressed with valid alignments.)
    buffer.Store<Block>(16, buffer.LoadAligned<Block>(4, 4), 16);
}

