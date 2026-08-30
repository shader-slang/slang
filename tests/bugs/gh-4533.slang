//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -capability cuda_sm_7_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// CHECK:      0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    vector<float,1> k = float1(tid);
    outputBuffer[tid] = all(k) && any(k) && bool(asint(k)) && bool(asuint(k)) && bool(sign(k));
}
