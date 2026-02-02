//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// CHECK: 2
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 16448
// CHECK: 85

[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = sizeof(BFloat16);
    uint64_t bitsBF16 = 0x4080404040003F80; // bfloat16 (1.0, 2.0, 3.0, 4.0)
    let vecBF16 = bit_cast<vector<BFloat16, 4>>(bitsBF16);
    let v4f = float4(vecBF16);
    outputBuffer[1] = (int)(v4f.x);
    outputBuffer[2] = (int)(v4f.y);
    outputBuffer[3] = (int)(v4f.z);
    outputBuffer[4] = (int)(v4f.w);

    BFloat16 valBF16 = BFloat16(3.0);
    outputBuffer[5] = bit_cast<uint16_t>(valBF16);
    __target_switch
    {
    case cuda:
        vector<BFloat16, 4> vecBF16_2 = vector<BFloat16, 4>(BFloat16(0.5), BFloat16(0.5), BFloat16(1.0), BFloat16(1.0));
        BFloat16 dotResult = dot(vecBF16, vecBF16_2);
        outputBuffer[6] = (int)((float)(dotResult) * 10);
    default:
        // Vulkan/NVIDIA currently does not support dot product.
        outputBuffer[6] = 85;
    }
}
