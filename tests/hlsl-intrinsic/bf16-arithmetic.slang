//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Scalar arithmetic: a=3.0, b=2.0
// CHECK: 50
// CHECK: 10
// CHECK: 60
// CHECK: 15
// CHECK: -30

// Vector arithmetic: va=(3.0, 4.0), vb=(2.0, 1.0)
// CHECK: 50
// CHECK: 30
// CHECK: 60
// CHECK: 40
// CHECK: -30

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(3.0);
    BFloat16 b = BFloat16(2.0);

    BFloat16 sum = a + b;
    BFloat16 diff = a - b;
    BFloat16 prod = a * b;
    BFloat16 quot = a / b;
    BFloat16 neg = -a;

    outputBuffer[0] = int(float(sum) * 10.0);
    outputBuffer[1] = int(float(diff) * 10.0);
    outputBuffer[2] = int(float(prod) * 10.0);
    outputBuffer[3] = int(float(quot) * 10.0);
    outputBuffer[4] = int(float(neg) * 10.0);

    // Vector arithmetic
    vector<BFloat16, 2> va = vector<BFloat16, 2>(BFloat16(3.0), BFloat16(4.0));
    vector<BFloat16, 2> vb = vector<BFloat16, 2>(BFloat16(2.0), BFloat16(1.0));

    let vsum = va + vb;
    let vdiff = va - vb;
    let vprod = va * vb;
    let vquot = va / vb;
    let vneg = -va;

    outputBuffer[5] = int(float(vsum[0]) * 10.0);
    outputBuffer[6] = int(float(vdiff[1]) * 10.0);
    outputBuffer[7] = int(float(vprod[0]) * 10.0);
    outputBuffer[8] = int(float(vquot[1]) * 10.0);
    outputBuffer[9] = int(float(vneg[0]) * 10.0);
}
