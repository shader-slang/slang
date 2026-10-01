// Direct collected-global vectors preserve all lane bits and neighboring scalar/resource fields.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT:uniform(data=[73]):name=prefix
uniform uint prefix;
//TEST_INPUT:uniform(data=[-7 11]):name=i2
uniform int2 i2;
//TEST_INPUT:uniform(data=[-13 17 -19]):name=i3
uniform int3 i3;
//TEST_INPUT:uniform(data=[79]):name=guardi
uniform uint guardi;
//TEST_INPUT:ubuffer(data=[97], stride=4):name=firstResource
StructuredBuffer<uint> firstResource;
//TEST_INPUT:uniform(data=[23 -29 31 -37]):name=i4
uniform int4 i4;
//TEST_INPUT:uniform(data=[2147483649 4000000001]):name=u2
uniform uint2 u2;
//TEST_INPUT:uniform(data=[41 2147483691 47]):name=u3
uniform uint3 u3;
//TEST_INPUT:uniform(data=[83]):name=guardu
uniform uint guardu;
//TEST_INPUT:ubuffer(data=[101], stride=4):name=secondResource
StructuredBuffer<uint> secondResource;
//TEST_INPUT:uniform(data=[53 59 61 67]):name=u4
uniform uint4 u4;
//TEST_INPUT:uniform(data=[-0.0 1.5]):name=f2
uniform float2 f2;
//TEST_INPUT:uniform(data=[-2.25 3.125 4.5]):name=f3
uniform float3 f3;
//TEST_INPUT:uniform(data=[89]):name=guardf
uniform uint guardf;
//TEST_INPUT:uniform(data=[5.75 -6.5 7.25 -8.125]):name=f4
uniform float4 f4;
//TEST_INPUT:ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
T retainVector<T>(T value) { return value; }

[numthreads(1, 1, 1)]
void computeMain()
{
    let loadedi2 = retainVector(i2);
    outputBuffer[1] = asuint(loadedi2.x);
    outputBuffer[2] = asuint(loadedi2.y);
    let loadedi3 = retainVector(i3);
    outputBuffer[3] = asuint(loadedi3.x);
    outputBuffer[4] = asuint(loadedi3.y);
    outputBuffer[5] = asuint(loadedi3.z);
    let loadedi4 = retainVector(i4);
    outputBuffer[6] = asuint(loadedi4.x);
    outputBuffer[7] = asuint(loadedi4.y);
    outputBuffer[8] = asuint(loadedi4.z);
    outputBuffer[9] = asuint(loadedi4.w);
    let loadedu2 = retainVector(u2);
    outputBuffer[10] = asuint(loadedu2.x);
    outputBuffer[11] = asuint(loadedu2.y);
    let loadedu3 = retainVector(u3);
    outputBuffer[12] = asuint(loadedu3.x);
    outputBuffer[13] = asuint(loadedu3.y);
    outputBuffer[14] = asuint(loadedu3.z);
    let loadedu4 = retainVector(u4);
    outputBuffer[15] = asuint(loadedu4.x);
    outputBuffer[16] = asuint(loadedu4.y);
    outputBuffer[17] = asuint(loadedu4.z);
    outputBuffer[18] = asuint(loadedu4.w);
    let loadedf2 = retainVector(f2);
    outputBuffer[19] = asuint(loadedf2.x);
    outputBuffer[20] = asuint(loadedf2.y);
    let loadedf3 = retainVector(f3);
    outputBuffer[21] = asuint(loadedf3.x);
    outputBuffer[22] = asuint(loadedf3.y);
    outputBuffer[23] = asuint(loadedf3.z);
    let loadedf4 = retainVector(f4);
    outputBuffer[24] = asuint(loadedf4.x);
    outputBuffer[25] = asuint(loadedf4.y);
    outputBuffer[26] = asuint(loadedf4.z);
    outputBuffer[27] = asuint(loadedf4.w);
    outputBuffer[28] = guardi;
    outputBuffer[29] = guardu;
    outputBuffer[30] = guardf;
    outputBuffer[31] = firstResource[0];
    outputBuffer[32] = secondResource[0];
    outputBuffer[33] = prefix;
    outputBuffer[34] = 4096;
}

//CHECK: {{^}}type: uint32_t{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
//CHECK-NEXT: {{^}}4294967289{{$}}
//CHECK-NEXT: {{^}}11{{$}}
//CHECK-NEXT: {{^}}4294967283{{$}}
//CHECK-NEXT: {{^}}17{{$}}
//CHECK-NEXT: {{^}}4294967277{{$}}
//CHECK-NEXT: {{^}}23{{$}}
//CHECK-NEXT: {{^}}4294967267{{$}}
//CHECK-NEXT: {{^}}31{{$}}
//CHECK-NEXT: {{^}}4294967259{{$}}
//CHECK-NEXT: {{^}}2147483649{{$}}
//CHECK-NEXT: {{^}}4000000001{{$}}
//CHECK-NEXT: {{^}}41{{$}}
//CHECK-NEXT: {{^}}2147483691{{$}}
//CHECK-NEXT: {{^}}47{{$}}
//CHECK-NEXT: {{^}}53{{$}}
//CHECK-NEXT: {{^}}59{{$}}
//CHECK-NEXT: {{^}}61{{$}}
//CHECK-NEXT: {{^}}67{{$}}
//CHECK-NEXT: {{^}}2147483648{{$}}
//CHECK-NEXT: {{^}}1069547520{{$}}
//CHECK-NEXT: {{^}}3222274048{{$}}
//CHECK-NEXT: {{^}}1078460416{{$}}
//CHECK-NEXT: {{^}}1083179008{{$}}
//CHECK-NEXT: {{^}}1085800448{{$}}
//CHECK-NEXT: {{^}}3234856960{{$}}
//CHECK-NEXT: {{^}}1088946176{{$}}
//CHECK-NEXT: {{^}}3238133760{{$}}
//CHECK-NEXT: {{^}}79{{$}}
//CHECK-NEXT: {{^}}83{{$}}
//CHECK-NEXT: {{^}}89{{$}}
//CHECK-NEXT: {{^}}97{{$}}
//CHECK-NEXT: {{^}}101{{$}}
//CHECK-NEXT: {{^}}73{{$}}
//CHECK-NEXT: {{^}}4096{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
