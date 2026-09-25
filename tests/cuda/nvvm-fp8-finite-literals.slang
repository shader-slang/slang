//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Finite FP8 literals fold completely before backend admission. Expectations are exact
// Float32 bits from the representable-value grid, including both sides of tie parity.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = asuint(float(FloatE4M3(0.00000000000000000000f)));
    outputBuffer[1] = asuint(float(FloatE4M3(0.00195312500000000000f)));
    outputBuffer[2] = asuint(float(FloatE4M3(0.01367187500000000000f)));
    outputBuffer[3] = asuint(float(FloatE4M3(0.01562500000000000000f)));
    outputBuffer[4] = asuint(float(FloatE4M3(256.00000000000000000000f)));
    outputBuffer[5] = asuint(float(FloatE4M3(288.00000000000000000000f)));
    outputBuffer[6] = asuint(float(FloatE4M3(320.00000000000000000000f)));
    outputBuffer[7] = asuint(float(FloatE4M3(352.00000000000000000000f)));
    outputBuffer[8] = asuint(float(FloatE4M3(384.00000000000000000000f)));
    outputBuffer[9] = asuint(float(FloatE4M3(416.00000000000000000000f)));
    outputBuffer[10] = asuint(float(FloatE4M3(448.00000000000000000000f)));
    outputBuffer[11] = asuint(float(FloatE4M3(0.00097656250000000000f)));
    outputBuffer[12] = asuint(float(FloatE4M3(0.00292968750000000000f)));
    outputBuffer[13] = asuint(float(FloatE4M3(0.01464843750000000000f)));
    outputBuffer[14] = asuint(float(FloatE4M3(1.06250000000000000000f)));
    outputBuffer[15] = asuint(float(FloatE4M3(1.18750000000000000000f)));
    outputBuffer[16] = asuint(float(FloatE4M3(-0.00000000000000000000f)));
    outputBuffer[17] = asuint(float(FloatE4M3(-0.00195312500000000000f)));
    outputBuffer[18] = asuint(float(FloatE4M3(-0.01367187500000000000f)));
    outputBuffer[19] = asuint(float(FloatE4M3(-0.01562500000000000000f)));
    outputBuffer[20] = asuint(float(FloatE4M3(-256.00000000000000000000f)));
    outputBuffer[21] = asuint(float(FloatE4M3(-288.00000000000000000000f)));
    outputBuffer[22] = asuint(float(FloatE4M3(-320.00000000000000000000f)));
    outputBuffer[23] = asuint(float(FloatE4M3(-352.00000000000000000000f)));
    outputBuffer[24] = asuint(float(FloatE4M3(-384.00000000000000000000f)));
    outputBuffer[25] = asuint(float(FloatE4M3(-416.00000000000000000000f)));
    outputBuffer[26] = asuint(float(FloatE4M3(-448.00000000000000000000f)));
    outputBuffer[27] = asuint(float(FloatE4M3(-0.00097656250000000000f)));
    outputBuffer[28] = asuint(float(FloatE4M3(-0.00292968750000000000f)));
    outputBuffer[29] = asuint(float(FloatE4M3(-0.01464843750000000000f)));
    outputBuffer[30] = asuint(float(FloatE4M3(-1.06250000000000000000f)));
    outputBuffer[31] = asuint(float(FloatE4M3(-1.18750000000000000000f)));
    outputBuffer[32] = asuint(float(FloatE5M2(0.00000000000000000000f)));
    outputBuffer[33] = asuint(float(FloatE5M2(0.00001525878906250000f)));
    outputBuffer[34] = asuint(float(FloatE5M2(0.00004577636718750000f)));
    outputBuffer[35] = asuint(float(FloatE5M2(0.00006103515625000000f)));
    outputBuffer[36] = asuint(float(FloatE5M2(57344.00000000000000000000f)));
    outputBuffer[37] = asuint(float(FloatE5M2(0.00000762939453125000f)));
    outputBuffer[38] = asuint(float(FloatE5M2(0.00002288818359375000f)));
    outputBuffer[39] = asuint(float(FloatE5M2(0.00005340576171875000f)));
    outputBuffer[40] = asuint(float(FloatE5M2(1.12500000000000000000f)));
    outputBuffer[41] = asuint(float(FloatE5M2(1.37500000000000000000f)));
    outputBuffer[42] = asuint(float(FloatE5M2(-0.00000000000000000000f)));
    outputBuffer[43] = asuint(float(FloatE5M2(-0.00001525878906250000f)));
    outputBuffer[44] = asuint(float(FloatE5M2(-0.00004577636718750000f)));
    outputBuffer[45] = asuint(float(FloatE5M2(-0.00006103515625000000f)));
    outputBuffer[46] = asuint(float(FloatE5M2(-57344.00000000000000000000f)));
    outputBuffer[47] = asuint(float(FloatE5M2(-0.00000762939453125000f)));
    outputBuffer[48] = asuint(float(FloatE5M2(-0.00002288818359375000f)));
    outputBuffer[49] = asuint(float(FloatE5M2(-0.00005340576171875000f)));
    outputBuffer[50] = asuint(float(FloatE5M2(-1.12500000000000000000f)));
    outputBuffer[51] = asuint(float(FloatE5M2(-1.37500000000000000000f)));
}

//CHECK: type: uint
//CHECK-NEXT: 0
//CHECK-NEXT: 989855744
//CHECK-NEXT: 1012924416
//CHECK-NEXT: 1015021568
//CHECK-NEXT: 1132462080
//CHECK-NEXT: 1133510656
//CHECK-NEXT: 1134559232
//CHECK-NEXT: 1135607808
//CHECK-NEXT: 1136656384
//CHECK-NEXT: 1137704960
//CHECK-NEXT: 1138753536
//CHECK-NEXT: 0
//CHECK-NEXT: 998244352
//CHECK-NEXT: 1015021568
//CHECK-NEXT: 1065353216
//CHECK-NEXT: 1067450368
//CHECK-NEXT: 2147483648
//CHECK-NEXT: 3137339392
//CHECK-NEXT: 3160408064
//CHECK-NEXT: 3162505216
//CHECK-NEXT: 3279945728
//CHECK-NEXT: 3280994304
//CHECK-NEXT: 3282042880
//CHECK-NEXT: 3283091456
//CHECK-NEXT: 3284140032
//CHECK-NEXT: 3285188608
//CHECK-NEXT: 3286237184
//CHECK-NEXT: 2147483648
//CHECK-NEXT: 3145728000
//CHECK-NEXT: 3162505216
//CHECK-NEXT: 3212836864
//CHECK-NEXT: 3214934016
//CHECK-NEXT: 0
//CHECK-NEXT: 931135488
//CHECK-NEXT: 943718400
//CHECK-NEXT: 947912704
//CHECK-NEXT: 1197473792
//CHECK-NEXT: 0
//CHECK-NEXT: 939524096
//CHECK-NEXT: 947912704
//CHECK-NEXT: 1065353216
//CHECK-NEXT: 1069547520
//CHECK-NEXT: 2147483648
//CHECK-NEXT: 3078619136
//CHECK-NEXT: 3091202048
//CHECK-NEXT: 3095396352
//CHECK-NEXT: 3344957440
//CHECK-NEXT: 2147483648
//CHECK-NEXT: 3087007744
//CHECK-NEXT: 3095396352
//CHECK-NEXT: 3212836864
//CHECK-NEXT: 3217031168
