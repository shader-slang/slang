//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type -render-features bfloat16
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -render-features bfloat16

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Scalar comparisons: a=3.0, b=2.0, c=3.0
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 1
// CHECK: 0
// CHECK: 1

// Vector comparisons: va=(3.0, 2.0), vb=(2.0, 2.0)
// CHECK: 0
// CHECK: 1
// CHECK: 0
// CHECK: 1
// CHECK: 1
// CHECK: 1

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(3.0);
    BFloat16 b = BFloat16(2.0);
    BFloat16 c = BFloat16(3.0);

    outputBuffer[0] = int(a == c);
    outputBuffer[1] = int(a == b);
    outputBuffer[2] = int(a != b);
    outputBuffer[3] = int(a != c);
    outputBuffer[4] = int(a > b);
    outputBuffer[5] = int(a >= c);
    outputBuffer[6] = int(a < b);
    outputBuffer[7] = int(b <= a);

    // Vector comparisons
    vector<BFloat16, 2> va = vector<BFloat16, 2>(BFloat16(3.0), BFloat16(2.0));
    vector<BFloat16, 2> vb = vector<BFloat16, 2>(BFloat16(2.0), BFloat16(2.0));

    let veq = (va == vb);
    let vneq = (va != vb);
    let vgt = (va > vb);
    let vge = (va >= vb);
    let vlt = (va < vb);
    let vle = (va <= vb);

    outputBuffer[8]  = int(veq[0]);
    outputBuffer[9]  = int(vneq[0]);
    outputBuffer[10] = int(vlt[0]);
    outputBuffer[11] = int(vgt[0]);
    outputBuffer[12] = int(vge[1]);
    outputBuffer[13] = int(vle[1]);
}
