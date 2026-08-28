//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -shaderobj -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=CHECK_NVVM): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o -

// Test that __getStructuredBufferPtr and __getByteAddressBufferPtr work on CUDA.
// Ptr<T[]> emits as T** on CUDA, and the buffer's .data pointer is used.

//TEST_INPUT:ubuffer(data=[10 20 30 40], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[100 200 300 400], stride=4):name=inputBytes
RWByteAddressBuffer inputBytes;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let sptr = __getStructuredBufferPtr(inputBuffer);
    outputBuffer[tid.x] = (*sptr)[tid.x] + 1;

    let bptr = __getByteAddressBufferPtr(inputBytes);
    outputBuffer[tid.x + 4] = int((*bptr)[tid.x]) + 2;
}

// CHECK: 11
// CHECK: 21
// CHECK: 31
// CHECK: 41
// CHECK: 102
// CHECK: 202
// CHECK: 302
// CHECK: 402

// The direct route reconstructs the three raw buffer views from the conventional global-parameter
// block, then indexes the exposed structured and byte-address data pointers as ordinary global
// uint32 pointers. The runtime lane above verifies that both stores preserve the CUDA result.
// CHECK_NVVM: .visible .const .align 8 .b8 SLANG_globalParams[48]
// CHECK_NVVM: .visible .entry computeMain()
// CHECK_NVVM-DAG: ld.const.u64 {{.*}}, [SLANG_globalParams];
// CHECK_NVVM-DAG: ld.const.u64 {{.*}}, [SLANG_globalParams+16];
// CHECK_NVVM-DAG: ld.const.u64 {{.*}}, [SLANG_globalParams+32];
// CHECK_NVVM-DAG: ld.global.u32
// CHECK_NVVM-DAG: ld.global.u32
// CHECK_NVVM-DAG: st.global.u32
// CHECK_NVVM-DAG: st.global.u32
