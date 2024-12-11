//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compile-arg -skip-spirv-validation -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu

// CHECK: 8765432F
// CHECK-NEXT: 876543F1
// CHECK-NEXT: 87654F21
// CHECK-NEXT: 8765F321
// CHECK-NEXT: A8
// CHECK-NEXT: 3FC0
// CHECK-NEXT: 123456F8
// CHECK-NEXT: 9ABCDE60
// CHECK-NEXT: 87654331
// CHECK-NEXT: FEDCBA68


//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9], stride=4):out,name=i16Buffer
RWStructuredBuffer<uint> i16Buffer;

//TEST_INPUT:ubuffer(data=[0 1 2 3 4 5 6 7 8 9], stride=4):out,name=i32Buffer
RWStructuredBuffer<uint> i32Buffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=i64Buffer
RWStructuredBuffer<uint64_t> i64Buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
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
}
