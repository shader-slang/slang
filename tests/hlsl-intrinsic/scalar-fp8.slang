//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type -render-features fp8
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// CHECK: 1
// CHECK: 1
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 68
// CHECK: 66
[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = sizeof(FloatE4M3);
    outputBuffer[1] = sizeof(FloatE5M2);
    uint bitsE4M3 = 0x48444038; // e4m3 (1.0, 2.0, 3.0, 4.0)
    vector<FloatE4M3, 4> vecE4M3 = bit_cast<vector<FloatE4M3, 4>>(bitsE4M3);
    outputBuffer[2] = (int)(float)(vecE4M3.x);
    outputBuffer[3] = (int)(float)(vecE4M3.y);
    outputBuffer[4] = (int)(float)(vecE4M3.z);
    outputBuffer[5] = (int)(float)(vecE4M3.w);

    uint bitsE5M2 = 0x4442403C; // e5m2 (1.0, 2.0, 3.0, 4.0)
    vector<FloatE5M2, 4> vecE5M2 = bit_cast<vector<FloatE5M2, 4>>(bitsE5M2);
    float4 vec_f = float4(vecE5M2);
    outputBuffer[6] = (int)(vec_f.x);
    outputBuffer[7] = (int)(vec_f.y);
    outputBuffer[8] = (int)(vec_f.z);
    outputBuffer[9] = (int)(vec_f.w);

    FloatE4M3 valE4M3 = FloatE4M3(3.0);
    outputBuffer[10] = bit_cast<uint8_t>(valE4M3);

    FloatE5M2 valE5M2 = FloatE5M2(3.0);
    outputBuffer[11] = bit_cast<uint8_t>(valE5M2);
}
