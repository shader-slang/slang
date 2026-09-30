// Exact independent IEEE input/oracle constants; no target-dependent oracle selection.
// Ceil rounds toward +infinity, floor toward -infinity, trunc toward zero.
// Preserve signed zeros and infinities; compare NaNs by class only.
// Each output mask reserves six bits per operation: scalar, helper, vector2/3/4, matrix2x2.
// 64 live lanes exercise 45 results each; completion words and end guards are checked.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 1022 0 1023 0 1024 0 1025 0 13312 0 14335 0 14336 0 14337 0 15359 0 15360 0 15361 0 15871 0 15872 0 15873 0 16383 0 16384 0 16385 0 16639 0 16640 0 16641 0 24575 0 24576 0 24577 0 25599 0 25600 0 25601 0 31743 0 31744 0 32547 0 31745 0 32768 0 32769 0 33790 0 33791 0 33792 0 33793 0 46080 0 47103 0 47104 0 47105 0 48127 0 48128 0 48129 0 48639 0 48640 0 48641 0 49151 0 49152 0 49153 0 49407 0 49408 0 49409 0 57343 0 57344 0 57345 0 58367 0 58368 0 58369 0 64511 0 64512 0 65315 0 64513 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 0 0 0 0 0 0 15360 0 0 15360 0 0 15360 0 0 16384 0 0 15360 0 0 15360 0 0 16384 0 0 15360 0 0 15360 0 0 16384 0 0 15360 0 0 15360 0 0 16384 0 0 15360 0 0 15360 0 0 16384 0 0 15360 0 0 15360 0 0 16384 0 0 16384 0 0 16384 0 0 16896 0 0 16384 0 0 16384 0 0 16896 0 0 16384 0 0 16384 0 0 16896 0 0 16384 0 0 16384 0 0 16896 0 0 16384 0 0 16384 0 0 24576 0 0 24572 0 0 24572 0 0 24576 0 0 24576 0 0 24576 0 0 24578 0 0 24576 0 0 24576 0 0 25600 0 0 25598 0 0 25598 0 0 25600 0 0 25600 0 0 25600 0 0 25601 0 0 25601 0 0 25601 0 0 31743 0 0 31743 0 0 31743 0 0 31744 0 0 31744 0 0 31744 0 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 32768 0 0 32768 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 32768 0 0 48128 0 0 32768 0 0 48128 0 0 48128 0 0 48128 0 0 48128 0 0 49152 0 0 48128 0 0 48128 0 0 49152 0 0 48128 0 0 48128 0 0 49152 0 0 48128 0 0 48128 0 0 49152 0 0 48128 0 0 48128 0 0 49152 0 0 48128 0 0 49152 0 0 49152 0 0 49152 0 0 49152 0 0 49664 0 0 49152 0 0 49152 0 0 49664 0 0 49152 0 0 49152 0 0 49664 0 0 49152 0 0 49152 0 0 49664 0 0 49152 0 0 57340 0 0 57344 0 0 57340 0 0 57344 0 0 57344 0 0 57344 0 0 57344 0 0 57346 0 0 57344 0 0 58366 0 0 58368 0 0 58366 0 0 58368 0 0 58368 0 0 58368 0 0 58369 0 0 58369 0 0 58369 0 0 64511 0 0 64511 0 0 64511 0 0 64512 0 0 64512 0 0 64512 0 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

half loadValue(uint lane)
{
    return bit_cast<half>(uint16_t(inputWords[2 * lane]));
}

bool matchesExpected(half value, uint lane, uint operation)
{
    uint low = uint(bit_cast<uint16_t>(value));
    uint high = 0;
    bool nanBits = (low & 0x7c00u) == 0x7c00u && (low & 0x3ffu) != 0;
    uint offset = 9 * lane + 3 * operation;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
half ceilThroughHelper(half value)
{
    return ceil(value);
}

[noinline]
half floorThroughHelper(half value)
{
    return floor(value);
}

[noinline]
half truncThroughHelper(half value)
{
    return trunc(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    errors |= matchesExpected(ceil(loadValue(lane)), lane, 0) ? 0u : 1u;
    errors |= matchesExpected(ceilThroughHelper(loadValue(lane)), lane, 0) ? 0u : 2u;
    half2 ceilResult2 = ceil(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(ceilResult2.x, (lane + 0) % 64, 0) ? 0u : 4u;
    errors |= matchesExpected(ceilResult2.y, (lane + 1) % 64, 0) ? 0u : 4u;
    half3 ceilResult3 = ceil(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(ceilResult3.x, (lane + 0) % 64, 0) ? 0u : 8u;
    errors |= matchesExpected(ceilResult3.y, (lane + 1) % 64, 0) ? 0u : 8u;
    errors |= matchesExpected(ceilResult3.z, (lane + 2) % 64, 0) ? 0u : 8u;
    half4 ceilResult4 = ceil(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(ceilResult4.x, (lane + 0) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.y, (lane + 1) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.z, (lane + 2) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.w, (lane + 3) % 64, 0) ? 0u : 16u;
    matrix<half, 2, 2> ceilMatrix = ceil(matrix<half, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(ceilMatrix[0][0], (lane + 0) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[0][1], (lane + 1) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[1][0], (lane + 2) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[1][1], (lane + 3) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(floor(loadValue(lane)), lane, 1) ? 0u : 64u;
    errors |= matchesExpected(floorThroughHelper(loadValue(lane)), lane, 1) ? 0u : 128u;
    half2 floorResult2 = floor(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(floorResult2.x, (lane + 0) % 64, 1) ? 0u : 256u;
    errors |= matchesExpected(floorResult2.y, (lane + 1) % 64, 1) ? 0u : 256u;
    half3 floorResult3 = floor(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(floorResult3.x, (lane + 0) % 64, 1) ? 0u : 512u;
    errors |= matchesExpected(floorResult3.y, (lane + 1) % 64, 1) ? 0u : 512u;
    errors |= matchesExpected(floorResult3.z, (lane + 2) % 64, 1) ? 0u : 512u;
    half4 floorResult4 = floor(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(floorResult4.x, (lane + 0) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.y, (lane + 1) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.z, (lane + 2) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.w, (lane + 3) % 64, 1) ? 0u : 1024u;
    matrix<half, 2, 2> floorMatrix = floor(matrix<half, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(floorMatrix[0][0], (lane + 0) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[0][1], (lane + 1) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[1][0], (lane + 2) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[1][1], (lane + 3) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(trunc(loadValue(lane)), lane, 2) ? 0u : 4096u;
    errors |= matchesExpected(truncThroughHelper(loadValue(lane)), lane, 2) ? 0u : 8192u;
    half2 truncResult2 = trunc(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(truncResult2.x, (lane + 0) % 64, 2) ? 0u : 16384u;
    errors |= matchesExpected(truncResult2.y, (lane + 1) % 64, 2) ? 0u : 16384u;
    half3 truncResult3 = trunc(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(truncResult3.x, (lane + 0) % 64, 2) ? 0u : 32768u;
    errors |= matchesExpected(truncResult3.y, (lane + 1) % 64, 2) ? 0u : 32768u;
    errors |= matchesExpected(truncResult3.z, (lane + 2) % 64, 2) ? 0u : 32768u;
    half4 truncResult4 = trunc(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(truncResult4.x, (lane + 0) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.y, (lane + 1) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.z, (lane + 2) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.w, (lane + 3) % 64, 2) ? 0u : 65536u;
    matrix<half, 2, 2> truncMatrix = trunc(matrix<half, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(truncMatrix[0][0], (lane + 0) % 64, 2) ? 0u : 131072u;
    errors |= matchesExpected(truncMatrix[0][1], (lane + 1) % 64, 2) ? 0u : 131072u;
    errors |= matchesExpected(truncMatrix[1][0], (lane + 2) % 64, 2) ? 0u : 131072u;
    errors |= matchesExpected(truncMatrix[1][1], (lane + 3) % 64, 2) ? 0u : 131072u;
    outputBuffer[1 + 2 * lane] = errors;
    outputBuffer[2 + 2 * lane] = 16000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16061{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16062{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16063{{$}}
// CHECK-NEXT: 3735928559{{$}}
