//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -output-using-type -render-features bfloat16
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -render-features bfloat16

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Scalar arithmetic: a=3.0, b=2.0
// CHECK: 50
// CHECK: 10
// CHECK: 60
// CHECK: 15
// CHECK: -30

// Vector2 arithmetic: va=(3.0, 4.0), vb=(2.0, 1.0)
// CHECK: 50
// CHECK: 30
// CHECK: 60
// CHECK: 40
// CHECK: -30

// Scalar unary plus, compound assignment, and inc/dec
// CHECK: 30
// CHECK: 50
// CHECK: 30
// CHECK: 60
// CHECK: 30
// CHECK: 40
// CHECK: 40
// CHECK: 50
// CHECK: 40
// CHECK: 40
// CHECK: 30

// Vector2 unary plus, compound assignment, and inc/dec
// CHECK: 30
// CHECK: 50
// CHECK: 60
// CHECK: 50
// CHECK: 100
// CHECK: 200
// CHECK: 100
// CHECK: 110
// CHECK: 110
// CHECK: 100

// Vector4 arithmetic: v4a=(1.0, 2.0, 3.0, 4.0), v4b=(2.0, 1.0, 2.0, 1.0)
// sum=(3,3,5,5), diff=(-1,1,1,3), prod=(2,2,6,4), neg=(-1,-2,-3,-4)
// CHECK: 30
// CHECK: 10
// CHECK: 20
// CHECK: 50
// CHECK: 30
// CHECK: 50
// CHECK: 60
// CHECK: -40

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

    BFloat16 pos = +a;
    outputBuffer[10] = int(float(pos) * 10.0);

    BFloat16 acc = BFloat16(3.0);
    acc += b;
    outputBuffer[11] = int(float(acc) * 10.0);
    acc -= b;
    outputBuffer[12] = int(float(acc) * 10.0);
    acc *= b;
    outputBuffer[13] = int(float(acc) * 10.0);
    acc /= b;
    outputBuffer[14] = int(float(acc) * 10.0);

    BFloat16 preInc = ++acc;
    outputBuffer[15] = int(float(preInc) * 10.0);
    BFloat16 postInc = acc++;
    outputBuffer[16] = int(float(postInc) * 10.0);
    outputBuffer[17] = int(float(acc) * 10.0);
    BFloat16 preDec = --acc;
    outputBuffer[18] = int(float(preDec) * 10.0);
    BFloat16 postDec = acc--;
    outputBuffer[19] = int(float(postDec) * 10.0);
    outputBuffer[20] = int(float(acc) * 10.0);

    vector<BFloat16, 2> vacc = vector<BFloat16, 2>(a, BFloat16(4.0));
    let vpos = +vacc;
    outputBuffer[21] = int(float(vpos[0]) * 10.0);

    vacc += vb;
    outputBuffer[22] = int(float(vacc[0]) * 10.0);

    vacc += BFloat16(1.0);
    outputBuffer[23] = int(float(vacc[0]) * 10.0);

    vacc -= BFloat16(1.0);
    outputBuffer[24] = int(float(vacc[0]) * 10.0);

    vacc *= vb;
    outputBuffer[25] = int(float(vacc[0]) * 10.0);

    vacc *= BFloat16(2.0);
    outputBuffer[26] = int(float(vacc[0]) * 10.0);

    vacc /= BFloat16(2.0);
    outputBuffer[27] = int(float(vacc[0]) * 10.0);

    let vpreInc = ++vacc;
    outputBuffer[28] = int(float(vpreInc[0]) * 10.0);

    let vpostDec = vacc--;
    outputBuffer[29] = int(float(vpostDec[0]) * 10.0);
    outputBuffer[30] = int(float(vacc[0]) * 10.0);

    // Vector4 arithmetic
    vector<BFloat16, 4> v4a = vector<BFloat16, 4>(BFloat16(1.0), BFloat16(2.0), BFloat16(3.0), BFloat16(4.0));
    vector<BFloat16, 4> v4b = vector<BFloat16, 4>(BFloat16(2.0), BFloat16(1.0), BFloat16(2.0), BFloat16(1.0));

    let v4sum = v4a + v4b;
    let v4diff = v4a - v4b;
    let v4prod = v4a * v4b;
    let v4neg = -v4a;

    outputBuffer[31] = int(float(v4sum[0]) * 10.0);  // (1+2)*10 = 30
    outputBuffer[32] = int(float(v4diff[1]) * 10.0);  // (2-1)*10 = 10
    outputBuffer[33] = int(float(v4prod[0]) * 10.0);  // (1*2)*10 = 20
    outputBuffer[34] = int(float(v4sum[2]) * 10.0);   // (3+2)*10 = 50
    outputBuffer[35] = int(float(v4diff[3]) * 10.0);  // (4-1)*10 = 30
    outputBuffer[36] = int(float(v4sum[3]) * 10.0);   // (4+1)*10 = 50
    outputBuffer[37] = int(float(v4prod[2]) * 10.0);  // (3*2)*10 = 60
    outputBuffer[38] = int(float(v4neg[3]) * 10.0);   // -4*10 = -40
}
