//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compile-arg -skip-spirv-validation -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK:      1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 35
// CHECK-NEXT: BE
// CHECK-NEXT: FFFFFFFA
// CHECK-NEXT: A
// CHECK-NEXT: 23
// CHECK-NEXT: FFFFFFAB
// CHECK-NEXT: 76
// CHECK-NEXT: FFFFFFED

// CHECK-NEXT: 1
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

// CHECK-NEXT: 7654321A
// CHECK-NEXT: FEDCBA98
// CHECK-NEXT: 76543210
// CHECK-NEXT: FEDCBA9A
// CHECK-NEXT: 76543210
// CHECK-NEXT: FEDABA98
// CHECK-NEXT: 76543210
// CHECK-NEXT: AEDCBA98
// CHECK-NEXT: 654321F0
// CHECK-NEXT: 987
// CHECK-NEXT: 0
// CHECK-NEXT: F00
// CHECK-NEXT: 654321F0
// CHECK-NEXT: 987
// CHECK-NEXT: 654321F0
// CHECK-NEXT: 987
// CHECK-NEXT: 654321F0
// CHECK-NEXT: 987
// CHECK-NEXT: 654321F0
// CHECK-NEXT: 987

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name=i16Buffer
RWStructuredBuffer<int> i16Buffer;

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1], stride=4):out,name=i32Buffer
RWStructuredBuffer<int> i32Buffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=i64Buffer
RWStructuredBuffer<uint64_t> i64Buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // 16-bit tests
    {
        // Simple hex extraction to test, varying the offset.
        uint16_t value = 0x4321;
        i16Buffer[0] = bitfieldExtract(value, 4 * 0, 4);
        i16Buffer[1] = bitfieldExtract(value, 4 * 1, 4);
        i16Buffer[2] = bitfieldExtract(value, 4 * 2, 4);
        i16Buffer[3] = bitfieldExtract(value, 4 * 3, 4);

        // Now varying the bit length
        value = 0b001110111110110101;
        i16Buffer[4] = bitfieldExtract(value, 0, 6);
        i16Buffer[5] = bitfieldExtract(value, 6, 8);

        // Sign extension case
        //  - For unsigned data types, the most significant bits of the result will be set to zero.
        //  - For signed data types, the most significant bits will be set to the value of bit offset + base - 1
        //    (i.e., it is sign extended to the width of the return type).
        i16Buffer[6] = bitfieldExtract(int16_t(0b1010111), 3, 4);   // 0b1010 -> 0b1111111111111010
        i16Buffer[7] = bitfieldExtract(uint16_t(0b1010111u), 3, 4); // 0b1111 -> 0b0000000000001010

        // // Component-wise extraction
        vector<int16_t, 4> val4 = vector<int16_t, 4>(0x1234, 0x9abc, 0x8765, 0xfedc);
        vector<int16_t, 4> ext4 = bitfieldExtract(val4, 4, 8);
        i16Buffer[8] = ext4.x;
        i16Buffer[9] = ext4.y;
        i16Buffer[10] = ext4.z;
        i16Buffer[11] = ext4.w;
    }

    // 32-bit tests
    {
        // Simple hex extraction to test, varying the offset.
        uint value = 0x87654321;
        i32Buffer[0] = bitfieldExtract(value, 4 * 0, 4);
        i32Buffer[1] = bitfieldExtract(value, 4 * 1, 4);
        i32Buffer[2] = bitfieldExtract(value, 4 * 2, 4);
        i32Buffer[3] = bitfieldExtract(value, 4 * 3, 4);
        i32Buffer[4] = bitfieldExtract(value, 4 * 4, 4);
        i32Buffer[5] = bitfieldExtract(value, 4 * 5, 4);
        i32Buffer[6] = bitfieldExtract(value, 4 * 6, 4);
        i32Buffer[7] = bitfieldExtract(value, 4 * 7, 4);

        // Now varying the bit length
        value = 0b00111011111011110001111010100001;
        i32Buffer[8] = bitfieldExtract(value, 0, 6);
        i32Buffer[9] = bitfieldExtract(value, 6, 8);

        // Sign extension case
        //  - For unsigned data types, the most significant bits of the result will be set to zero.
        //  - For signed data types, the most significant bits will be set to the value of bit offset + base - 1
        //    (i.e., it is sign extended to the width of the return type).
        i32Buffer[10] = bitfieldExtract(0b1010111, 3, 4);  // 0b1010 -> 0b11111111111111111111111111111010
        i32Buffer[11] = bitfieldExtract(0b1010111u, 3, 4); // 0b1111 -> 0b00000000000000000000000000001010

        // Component-wise extraction
        int4 val4 = int4(0x12345678, 0x9abcdef0, 0x87654321, 0xfedcba98);
        int4 ext4 = bitfieldExtract(val4, 4, 8);
        i32Buffer[12] = ext4.x;
        i32Buffer[13] = ext4.y;
        i32Buffer[14] = ext4.z;
        i32Buffer[15] = ext4.w;
    }

    // 64-bit tests
    {
        // Simple hex insertion to test, varying the offset.
        uint64_t base = 0xFEDCBA9876543210ull;
        uint64_t insert = 0xAull;
        i64Buffer[0] = bitfieldInsert(base, insert, 4 * 0, 4);  // 0xFEDCBA987654321Aull -> 2271560495
        i64Buffer[1] = bitfieldInsert(base, insert, 4 * 8, 4);  // 0xFEDCBA98A6543210ull -> 2271560689
        i64Buffer[2] = bitfieldInsert(base, insert, 4 * 12, 4); // 0xFEDCAA9876543210ull -> 2271563553
        i64Buffer[3] = bitfieldInsert(base, insert, 4 * 15, 4); // 0xAEDCBA9876543210ull -> 2271605537

        // Test with varying bit length
        base = 0;
        insert = 0xFEDCBA987654321Full;
        i64Buffer[4] = bitfieldInsert(base, insert, 4, 40); // 0xA987654321 -> 16492674416640
        i64Buffer[5] = bitfieldInsert(base, insert, 40, 4); // 0xF000000000 -> 10477124133360

        // Test with a vector
        vector<uint64_t, 4> base4 = vector<uint64_t, 4>(base, base, base, base);
        vector<uint64_t, 4> insert4 = vector<uint64_t, 4>(insert, insert, insert, insert);
        vector<uint64_t, 4> output4 = bitfieldInsert(base4, insert4, 4, 40);
        i64Buffer[6] = output4.x;
        i64Buffer[7] = output4.y;
        i64Buffer[8] = output4.z;
        i64Buffer[9] = output4.w;
    }
}
