//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK:      8765432F
// CHECK-NEXT: 876543F1
// CHECK-NEXT: 87654F21
// CHECK-NEXT: 8765F321
// CHECK-NEXT: A8
// CHECK-NEXT: 3FC0
// CHECK-NEXT: 123456F8
// CHECK-NEXT: 9ABCDE60
// CHECK-NEXT: 87654331
// CHECK-NEXT: FEDCBA68

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Simple hex insertion to test, varying the offset.
    uint base = 0x87654321;
    uint value = 0xABCDEF;
    outputBuffer[0] = bitfieldInsert(base, value, 4 * 0, 4); // 0x8765432F -> 2271560495
    outputBuffer[1] = bitfieldInsert(base, value, 4 * 1, 4); // 0x876543F1 -> 2271560689
    outputBuffer[2] = bitfieldInsert(base, value, 4 * 2, 4); // 0x8765F321 -> 2271563553
    outputBuffer[3] = bitfieldInsert(base, value, 4 * 3, 4); // 0x87F54321 -> 2271605537

    // Test with varying bit length
    base = 0;
    value = 0b101010;
    outputBuffer[4] = bitfieldInsert(base, value, 2, 6); // 0b10101000 -> 1005526666
    value = 0b11111111;
    outputBuffer[5] = bitfieldInsert(base, value, 6, 8); // 0b11111111000000 -> 1005519841
    
    // Test with a vector
    uint4 base4 = uint4(0x12345678, 0x9abcdef0, 0x87654321, 0xfedcba98);
    uint4 value4 = uint4(0xABCDEF, 0x123456, 0x876543, 0x123456);
    uint4 output4 = bitfieldInsert(base4, value4, 4, 4);
    outputBuffer[6] = output4.x;
    outputBuffer[7] = output4.y;
    outputBuffer[8] = output4.z;
    outputBuffer[9] = output4.w;
}
