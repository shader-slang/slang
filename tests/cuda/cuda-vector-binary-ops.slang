// Guards the CUDA prelude's fixed-width vector binary and comparison operators. The prelude spells
// the components directly (`result.x = left.x op right.x; ...`) under preprocessor width dispatch;
// a mis-dispatched width or a wrong component would show up here as a wrong buffer value. Both
// backends are checked against the same expected results, and the CPU target uses a different
// prelude, so a CUDA-only regression fails while the CPU run keeps passing.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Negative operands and asymmetric values per component: a body that assigned the wrong
    // component, or reused one component's result for another, would not reproduce these outputs.
    int2 a2 = int2(7, -3);
    int2 b2 = int2(2, 5);

    int3 a3 = int3(1, 2, 3);
    int3 b3 = int3(4, 5, 6);

    int4 a4 = int4(1, 2, 3, 4);
    int4 b4 = int4(10, 20, 30, 40);

    // Arithmetic, bitwise and shift at width 2. Left-shifts use non-negative left operands only:
    // shifting a negative value left is undefined before C++20, so asserting a value for it would
    // make this test depend on the downstream compiler's choice rather than on the prelude.
    int2 shiftable2 = int2(7, 3);
    int2 sum2 = a2 + b2;
    int2 mul2 = a2 * b2;
    int2 and2 = a2 & b2;
    int2 shl2 = shiftable2 << b2;
    int2 shr2 = a2 >> b2;

    // Width 3 and 4 exercise the .z and .w arms of the width dispatch.
    int3 sum3 = a3 + b3;
    int4 sum4 = a4 + b4;
    int4 dif4 = b4 - a4;

    // A comparison whose result is mixed per component: a width-3 body that wrongly reused .y
    // for .z, or dropped .z entirely, would not reproduce this pattern.
    int3 mix3 = int3(1, 99, 3);
    bool3 eq3 = mix3 == a3;

    float3 f3 = float3(1.5, 2.5, 3.5) + float3(0.5, 0.5, 0.5);

    // 8-bit components with a negative operand are the one case where the operators' result depends
    // on how a component is read, not merely on which component: `int8_t2` lowers to CUDA's `char2`,
    // whose members are `signed char`, while the accessor path this change replaced reached them
    // through a plain `char*`. On a target where plain `char` is unsigned, that path yielded 125 for
    // `-6 >> 1` where the declared member type gives -3. Division, remainder and the comparisons
    // divide the same way; the additive and bitwise operators agree either way because the store
    // back into the component truncates the difference away.
    int8_t2 c2 = int8_t2(-6, 7);
    int8_t2 cshr2 = c2 >> int8_t2(1, 1);
    int8_t2 cdiv2 = c2 / int8_t2(2, 2);
    int8_t2 cmod2 = c2 % int8_t2(4, 4);
    bool2 cneg2 = c2 < int8_t2(0, 0);

    outputBuffer[0] = sum2.x;   //BUF: 9
    outputBuffer[1] = sum2.y;   //BUF-NEXT: 2
    outputBuffer[2] = mul2.x;   //BUF-NEXT: 14
    outputBuffer[3] = mul2.y;   //BUF-NEXT: -15
    outputBuffer[4] = and2.x;   //BUF-NEXT: 2
    outputBuffer[5] = and2.y;   //BUF-NEXT: 5
    outputBuffer[6] = shl2.x;   //BUF-NEXT: 28
    outputBuffer[7] = shl2.y;   //BUF-NEXT: 96
    outputBuffer[8] = shr2.x;   //BUF-NEXT: 1
    outputBuffer[9] = shr2.y;   //BUF-NEXT: -1

    outputBuffer[10] = sum3.x;  //BUF-NEXT: 5
    outputBuffer[11] = sum3.y;  //BUF-NEXT: 7
    outputBuffer[12] = sum3.z;  //BUF-NEXT: 9

    outputBuffer[13] = sum4.x;  //BUF-NEXT: 11
    outputBuffer[14] = sum4.y;  //BUF-NEXT: 22
    outputBuffer[15] = sum4.z;  //BUF-NEXT: 33
    outputBuffer[16] = sum4.w;  //BUF-NEXT: 44
    outputBuffer[17] = dif4.x;  //BUF-NEXT: 9
    outputBuffer[18] = dif4.y;  //BUF-NEXT: 18
    outputBuffer[19] = dif4.z;  //BUF-NEXT: 27
    outputBuffer[20] = dif4.w;  //BUF-NEXT: 36

    outputBuffer[21] = eq3.x ? 1 : 0;   //BUF-NEXT: 1
    outputBuffer[22] = eq3.y ? 1 : 0;   //BUF-NEXT: 0
    outputBuffer[23] = eq3.z ? 1 : 0;   //BUF-NEXT: 1

    outputBuffer[24] = int(f3.x * 2.0f); //BUF-NEXT: 4
    outputBuffer[25] = int(f3.z * 2.0f); //BUF-NEXT: 8

    outputBuffer[26] = int(cshr2.x);    //BUF-NEXT: -3
    outputBuffer[27] = int(cshr2.y);    //BUF-NEXT: 3
    outputBuffer[28] = int(cdiv2.x);    //BUF-NEXT: -3
    outputBuffer[29] = int(cdiv2.y);    //BUF-NEXT: 3
    outputBuffer[30] = int(cmod2.x);    //BUF-NEXT: -2
    outputBuffer[31] = int(cmod2.y);    //BUF-NEXT: 3
    outputBuffer[32] = cneg2.x ? 1 : 0; //BUF-NEXT: 1
    outputBuffer[33] = cneg2.y ? 1 : 0; //BUF-NEXT: 0
}
