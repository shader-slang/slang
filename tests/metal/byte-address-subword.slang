//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Metal byte-address buffers are arrays of 32-bit words, so scalar 8-bit and 16-bit
// accesses must extract or update part of a word. Each store below writes part of a word
// and the test reads the full word back to check that the neighboring bytes survive.

//TEST_INPUT:ubuffer(data=[0x3C004000 0x80017FFF 0x04030201 0x00000000], stride=4),name=src
ByteAddressBuffer src;

//TEST_INPUT:ubuffer(data=[0xdeadbeef 0xdeadbeef 0xdeadbeef 0xdeadbeef], stride=4),name=tmp
RWByteAddressBuffer tmp;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

// CHECK: type: uint32_t
// half 2.0 and 1.0, times 10
// CHECK-NEXT: 20
// CHECK-NEXT: 10
// int16_t 32767 and -32767
// CHECK-NEXT: 32767
// CHECK-NEXT: 4294934529
// uint8_t at byte offsets 9 and 11
// CHECK-NEXT: 2
// CHECK-NEXT: 4
// Words after Store<half>(2, 3.0), Store<uint8_t>(5, 0x55), Store<uint16_t>(8, 0x1234),
// and Store<int8_t>(15, -2)
// CHECK-NEXT: 1107345135
// CHECK-NEXT: 3735901679
// CHECK-NEXT: 3735884340
// CHECK-NEXT: 4272799471
// int8_t -2 read back from byte offset 15
// CHECK-NEXT: 4294967294

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = uint(float(src.Load<half>(0)) * 10.0);
    output[1] = uint(float(src.Load<half>(2)) * 10.0);
    output[2] = uint(int(src.Load<int16_t>(4)));
    output[3] = uint(int(src.Load<int16_t>(6)));
    output[4] = uint(src.Load<uint8_t>(9));
    output[5] = uint(src.Load<uint8_t>(11));

    tmp.Store<half>(2, half(3.0));
    tmp.Store<uint8_t>(5, uint8_t(0x55));
    tmp.Store<uint16_t>(8, uint16_t(0x1234));
    tmp.Store<int8_t>(15, int8_t(-2));
    output[6] = tmp.Load<uint>(0);
    output[7] = tmp.Load<uint>(4);
    output[8] = tmp.Load<uint>(8);
    output[9] = tmp.Load<uint>(12);
    output[10] = uint(int(tmp.Load<int8_t>(15)));
}
