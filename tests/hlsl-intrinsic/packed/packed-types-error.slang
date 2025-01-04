//TEST(compute):SIMPLE(filecheck=CHECK): -target spirv

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint8_t4_packed packedU1 = 0x0U;
    uint8_t4_packed packedU2 = 0xFU;
    int8_t4_packed packedS1 = 0xFU;
    int8_t4_packed packedS2 = 0xFU;

    // Arithmetic and logical (bitwise) operations are not supported on packed types.
    // An attempt to overload these operators will fail during compilation due to ambiguity caused by multiple possible overloads.

    // CHECK: error 39999: ambiguous call to '-' with arguments of type
    uint8_t4_packed val1 = packedU1 - packedU2;
    // CHECK: error 39999: ambiguous call to '*' with arguments of type
    int8_t4_packed val2 = packedS1 * packedS2;
    // CHECK: error 39999: ambiguous call to '&' with arguments of type
    uint8_t4_packed val3 = packedU1 & packedS1;
    // CHECK: error 39999: ambiguous call to '|' with arguments of type
    int8_t4_packed val4 = packedU1 | packedS1;
}
