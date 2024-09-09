//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK:      0
// CHECK-NEXT: 2
// CHECK-NEXT: 4
// CHECK-NEXT: 6
// CHECK-NEXT: 8
// CHECK-NEXT: 13
// CHECK-NEXT: 11
// CHECK-NEXT: 10
// CHECK-NEXT: 444691369455
// CHECK-NEXT: 10560325
// CHECK-NEXT: 18446744073709551610
// CHECK-NEXT: 10
// CHECK-NEXT: 86
// CHECK-NEXT: 18446744073709551582
// CHECK-NEXT: 67
// CHECK-NEXT: 18446744073709551546

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Simple hex extraction to test, varying the offset.
    uint64_t value = 0xABCDEF9876543210ull;
    outputBuffer[0] = bitfieldExtract(value, 4 * 0, 4);
    outputBuffer[1] = bitfieldExtract(value, 4 * 2, 4);
    outputBuffer[2] = bitfieldExtract(value, 4 * 4, 4);
    outputBuffer[3] = bitfieldExtract(value, 4 * 6, 4);
    outputBuffer[4] = bitfieldExtract(value, 4 * 8, 4);
    outputBuffer[5] = bitfieldExtract(value, 4 * 12, 4);
    outputBuffer[6] = bitfieldExtract(value, 4 * 14, 4);
    outputBuffer[7] = bitfieldExtract(value, 4 * 15, 4);

    // Now varying the bit length
    value = 0xA123456789ABCDEFull;
    outputBuffer[8] = bitfieldExtract(value, 0, 40);  // 0x6789ABCDEF
    outputBuffer[9] = bitfieldExtract(value, 40, 24); // 0xA12345

    // Sign extension case
    //  - For unsigned data types, the most significant bits of the result will be set to zero.
    //  - For signed data types, the most significant bits will be set to the value of bit offset + base - 1
    //    (i.e., it is sign extended to the width of the return type).
    outputBuffer[10] = bitfieldExtract(0b1010111ll, 3, 4);  // 0b1010 -> 0b11111111111111111111111111111010
    outputBuffer[11] = bitfieldExtract(0b1010111ull, 3, 4); // 0b1010 -> 0b00000000000000000000000000001010

    // Component-wise extraction
    int64_t4 val4 = int64_t4(0x123456789abcdef0ll, 0x9abcdef012345678ll, 0x87654321fedcba98ll, 0xfedcba9876543210ll);
    int64_t4 ext4 = bitfieldExtract(val4, 40, 8);
    outputBuffer[12] = ext4.x;
    outputBuffer[13] = ext4.y;
    outputBuffer[14] = ext4.z;
    outputBuffer[15] = ext4.w;
}
