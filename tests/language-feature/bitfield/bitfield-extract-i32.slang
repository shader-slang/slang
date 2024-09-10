//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK:      1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
// CHECK-NEXT: 21
// CHECK-NEXT: 7A
// CHECK-NEXT: FFFFFFFA
// CHECK-NEXT: A
// CHECK-NEXT: 67
// CHECK-NEXT: FFFFFFEF
// CHECK-NEXT: 32
// CHECK-NEXT: FFFFFFA9

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Simple hex extraction to test, varying the offset.
    uint value = 0x87654321;
    outputBuffer[0] = bitfieldExtract(value, 4 * 0, 4);
    outputBuffer[1] = bitfieldExtract(value, 4 * 1, 4);
    outputBuffer[2] = bitfieldExtract(value, 4 * 2, 4);
    outputBuffer[3] = bitfieldExtract(value, 4 * 3, 4);
    outputBuffer[4] = bitfieldExtract(value, 4 * 4, 4);
    outputBuffer[5] = bitfieldExtract(value, 4 * 5, 4);
    outputBuffer[6] = bitfieldExtract(value, 4 * 6, 4);
    outputBuffer[7] = bitfieldExtract(value, 4 * 7, 4);

    // Now varying the bit length
    value = 0b00111011111011110001111010100001;
    outputBuffer[8] = bitfieldExtract(value, 0, 6);
    outputBuffer[9] = bitfieldExtract(value, 6, 8);

    // Sign extension case
    //  - For unsigned data types, the most significant bits of the result will be set to zero.
    //  - For signed data types, the most significant bits will be set to the value of bit offset + base - 1
    //    (i.e., it is sign extended to the width of the return type).
    outputBuffer[10] = bitfieldExtract(0b1010111, 3, 4);  // 0b1010 -> 0b11111111111111111111111111111010
    outputBuffer[11] = bitfieldExtract(0b1010111u, 3, 4); // 0b1111 -> 0b00000000000000000000000000001010

    // Component-wise extraction
    int4 val4 = int4(0x12345678, 0x9abcdef0, 0x87654321, 0xfedcba98);
    int4 ext4 = bitfieldExtract(val4, 4, 8);
    outputBuffer[12] = ext4.x;
    outputBuffer[13] = ext4.y;
    outputBuffer[14] = ext4.z;
    outputBuffer[15] = ext4.w;
}
