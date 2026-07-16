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
    // The access below is valid, so no hard error is emitted. (The `LoadAligned<Block>(4, 4)`
    // call promises only 4-byte alignment, which is below `Block`'s 16-byte base alignment, so it
    // additionally emits the E41304 below-base warning — asserted in
    // byte-address-buffer-alignment-11545.slang, not here.)
    // CHECK-NOT: error

    // The single-argument `LoadAligned<Block>(0)` forwards `Block`'s base alignment (16 — the
    // element alignment of `float4[2]`), which is below the array's 16-byte-strided-per-element
    // *stride span* of 32, so the whole array is not loaded as one wide access; instead each
    // `float4` row loads wide at its 16-byte stride (two `Load<float4>`).
    // CHECK2: float4 {{.*}} = (buffer_0).Load<float4 >(0U);
    // CHECK2: float4 {{.*}} = (buffer_0).Load<float4 >(16U);
    // CHECK2: float4 {{.*}}[int(2)] = { {{.*}}, {{.*}} };
    // CHECK2: buffer_0.Store(0U,{{.*}});
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(4U);
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(8U);
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(12U);
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(16U);
    // CHECK2: float4 {{.*}} = float4({{.*}}, {{.*}}, {{.*}}, {{.*}});
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(20U);
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(24U);
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(28U);
    // CHECK2: float {{.*}} = (buffer_0).Load<float >(32U);
    // CHECK2: float4 {{.*}} = float4({{.*}}, {{.*}}, {{.*}}, {{.*}});
    // The 16-byte-aligned `Store<Block>(16, ..., 16)` promises only 16, below the array's 32-byte
    // stride, so the two reassembled rows are stored per-row rather than as one wide array store.
    // CHECK2: buffer_0.Store(16U,{{.*}});
    // CHECK2: buffer_0.Store(32U,{{.*}});

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

