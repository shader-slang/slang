//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

//CHECK:      7654321A
//CHECK-NEXT: FEDCBA98
//CHECK-NEXT: 76543210
//CHECK-NEXT: FEDCBA9A
//CHECK-NEXT: 76543210
//CHECK-NEXT: FEDABA98
//CHECK-NEXT: 76543210
//CHECK-NEXT: AEDCBA98
//CHECK-NEXT: 654321F0
//CHECK-NEXT: 987
//CHECK-NEXT: 0
//CHECK-NEXT: F00
//CHECK-NEXT: 654321F0
//CHECK-NEXT: 987
//CHECK-NEXT: 654321F0
//CHECK-NEXT: 987
//CHECK-NEXT: 654321F0
//CHECK-NEXT: 987
//CHECK-NEXT: 654321F0
//CHECK-NEXT: 987

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Simple hex insertion to test, varying the offset.
    uint64_t base = 0xFEDCBA9876543210ull;
    uint64_t insert = 0xAull;
    outputBuffer[0] = bitfieldInsert(base, insert, 4 * 0, 4);  // 0xFEDCBA987654321Aull -> 2271560495
    outputBuffer[1] = bitfieldInsert(base, insert, 4 * 8, 4);  // 0xFEDCBA98A6543210ull -> 2271560689
    outputBuffer[2] = bitfieldInsert(base, insert, 4 * 12, 4); // 0xFEDCAA9876543210ull -> 2271563553
    outputBuffer[3] = bitfieldInsert(base, insert, 4 * 15, 4); // 0xAEDCBA9876543210ull -> 2271605537

    // Test with varying bit length
    base = 0;
    insert = 0xFEDCBA987654321Full;
    outputBuffer[4] = bitfieldInsert(base, insert, 4, 40); // 0xA987654321 -> 16492674416640
    outputBuffer[5] = bitfieldInsert(base, insert, 40, 4); // 0xF000000000 -> 10477124133360

    // Test with a vector
    uint64_t4 base4 = uint64_t4(base, base, base, base);
    uint64_t4 insert4 = uint64_t4(insert, insert, insert, insert);
    uint64_t4 output4 = bitfieldInsert(base4, insert4, 4, 40);
    outputBuffer[6] = output4.x;
    outputBuffer[7] = output4.y;
    outputBuffer[8] = output4.z;
    outputBuffer[9] = output4.w;
}
