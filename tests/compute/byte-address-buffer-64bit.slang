// byte-address-buffer.slang

//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -shaderobj -profile cs_6_0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=CHECK_NVVM):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

//TEST_INPUT:ubuffer(data=[0 1 2 3]):name=inputBuffer
RWByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0]):out,name=outputBuffer
RWByteAddressBuffer outputBuffer;

void testInt64(uint val)
{
    // Load a 32-bit value from the input buffer
    uint tmp = inputBuffer.Load(uint(val * 4));

    // Cast to uint64_t
    uint64_t tmp64 = uint64_t(tmp) + 1;

    // Store the result back as uint64_t
    outputBuffer.Store(uint(val * 8), tmp64);
}

[numthreads(4, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    testInt64(tid);
}

// libNVVM preserves the 64-bit add, then lowers the four-byte-aligned UInt64 store to two ordered
// UInt stores in final PTX.
// CHECK_NVVM: .visible .const .align 8 .b8 SLANG_globalParams[32]
// CHECK_NVVM: .visible .entry computeMain()
// CHECK_NVVM: ld.global.u32
// CHECK_NVVM: add.s64
// CHECK_NVVM: st.global.u32
// CHECK_NVVM: shr.u64
// CHECK_NVVM: st.global.u32
