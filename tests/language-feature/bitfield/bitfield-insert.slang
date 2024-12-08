//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compile-arg -skip-spirv-validation -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK:      432D
// CHECK-NEXT: 43D1
// CHECK-NEXT: 4D21
// CHECK-NEXT: D321
// CHECK-NEXT: A8
// CHECK-NEXT: 3FC0
// CHECK-NEXT: 12D4
// CHECK-NEXT: 9A4C
// CHECK-NEXT: 8755
// CHECK-NEXT: FE4C

// CHECK-NEXT: 8765432F
// CHECK-NEXT: 876543F1
// CHECK-NEXT: 87654F21
// CHECK-NEXT: 8765F321
// CHECK-NEXT: A8
// CHECK-NEXT: 3FC0
// CHECK-NEXT: 123456F8
// CHECK-NEXT: 9ABCDE60
// CHECK-NEXT: 87654331
// CHECK-NEXT: FEDCBA68

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

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9], stride=4):out,name=i16Buffer
RWStructuredBuffer<uint> i16Buffer;

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9], stride=4):out,name=i32Buffer
RWStructuredBuffer<uint> i32Buffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=i64Buffer
RWStructuredBuffer<uint64_t> i64Buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // 16-bit tests
    {
        // Simple hex insertion to test, varying the offset.
        uint16_t base = 0x4321;
        uint16_t value = 0xABCD;
        i16Buffer[0] = bitfieldInsert(base, value, 4 * 0, 4); // 0x432D
        i16Buffer[1] = bitfieldInsert(base, value, 4 * 1, 4); // 0x43D1
        i16Buffer[2] = bitfieldInsert(base, value, 4 * 2, 4); // 0x4D21
        i16Buffer[3] = bitfieldInsert(base, value, 4 * 3, 4); // 0xD321

        // Test with varying bit length
        base = 0;
        value = 0b101010;
        i16Buffer[4] = bitfieldInsert(base, value, 2, 6); // 0b101010 00 -> 0xA8
        value = 0b11111111;
        i16Buffer[5] = bitfieldInsert(base, value, 6, 8); // 0b11111111 000000 -> 0x3FC0

        // Test with a vector
        vector<uint16_t, 4> base4 = vector<uint16_t, 4>(0x1234, 0x9abc, 0x8765, 0xfedc);
        vector<uint16_t, 4> value4 = vector<uint16_t, 4>(0xABCD, 0x1234, 0x8765, 0x1234);
        vector<uint16_t, 4> output4 = bitfieldInsert(base4, value4, 4, 4);
        i16Buffer[6] = output4.x;
        i16Buffer[7] = output4.y;
        i16Buffer[8] = output4.z;
        i16Buffer[9] = output4.w;
    }

    // 32-bit tests
    {
        // Simple hex insertion to test, varying the offset.
        uint base = 0x87654321;
        uint value = 0xABCDEF;
        i32Buffer[0] = bitfieldInsert(base, value, 4 * 0, 4); // 0x8765432F
        i32Buffer[1] = bitfieldInsert(base, value, 4 * 1, 4); // 0x876543F1
        i32Buffer[2] = bitfieldInsert(base, value, 4 * 2, 4); // 0x8765F321
        i32Buffer[3] = bitfieldInsert(base, value, 4 * 3, 4); // 0x87F54321

        // Test with varying bit length
        base = 0;
        value = 0b101010;
        i32Buffer[4] = bitfieldInsert(base, value, 2, 6); // 0b10101000
        value = 0b11111111;
        i32Buffer[5] = bitfieldInsert(base, value, 6, 8); // 0b11111111000000
        
        // Test with a vector
        uint4 base4 = uint4(0x12345678, 0x9abcdef0, 0x87654321, 0xfedcba98);
        uint4 value4 = uint4(0xABCDEF, 0x123456, 0x876543, 0x123456);
        uint4 output4 = bitfieldInsert(base4, value4, 4, 4);
        i32Buffer[6] = output4.x;
        i32Buffer[7] = output4.y;
        i32Buffer[8] = output4.z;
        i32Buffer[9] = output4.w;
    }

    // 64-bit tests
    {
        // Simple hex insertion to test, varying the offset.
        uint64_t base = 0xFEDCBA9876543210ull;
        uint64_t insert = 0xAull;
        i64Buffer[0] = bitfieldInsert(base, insert, 4 * 0, 4);  // 0xFEDCBA987654321Aull
        i64Buffer[1] = bitfieldInsert(base, insert, 4 * 8, 4);  // 0xFEDCBA98A6543210ull
        i64Buffer[2] = bitfieldInsert(base, insert, 4 * 12, 4); // 0xFEDCAA9876543210ull
        i64Buffer[3] = bitfieldInsert(base, insert, 4 * 15, 4); // 0xAEDCBA9876543210ull

        // Test with varying bit length
        base = 0;
        insert = 0xFEDCBA987654321Full;
        i64Buffer[4] = bitfieldInsert(base, insert, 4, 40); // 0xA987654321
        i64Buffer[5] = bitfieldInsert(base, insert, 40, 4); // 0xF000000000

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
