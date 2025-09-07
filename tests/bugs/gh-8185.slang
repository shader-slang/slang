//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -Xslang -DSKIP_HALF_PRECISION
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -profile cs_6_2 -compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -emit-spirv-directly -compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -Xslang -DSKIP_HALF_PRECISION
//TEST_DISABLED(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

void write_double(double f, int i)
{
    uint64_t v = bit_cast<uint64_t>(f);
    outputBuffer[i] = uint32_t(v);
    outputBuffer[i+1] = uint32_t(v >> 32);
}

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = bit_cast<uint32_t>(nextafter(-0.0f, 1.0f)); // CHECK: 1
    outputBuffer[1] = bit_cast<uint32_t>(nextafter(0.0f, 1.0f)); // CHECK-NEXT: 1
    outputBuffer[2] = bit_cast<uint32_t>(nextafter(-0.0f, -1.0f)); // CHECK-NEXT: 80000001
    outputBuffer[3] = bit_cast<uint32_t>(nextafter(0.0f, -1.0f)); // CHECK-NEXT: 80000001
    outputBuffer[4] = bit_cast<uint32_t>(nextafter(1000.0f, 2000.0f)); // CHECK-NEXT: 447A0001
    outputBuffer[5] = bit_cast<uint32_t>(nextafter(-1000.0f, -1.0f)); // CHECK-NEXT: C479FFFF

#ifdef SKIP_HALF_PRECISION
    outputBuffer[6] = 1;
    outputBuffer[7] = 1;
    outputBuffer[8] = 0x8001;
    outputBuffer[9] = 0x8001;
    outputBuffer[10] = 0x63D1;
    outputBuffer[11] = 0xE3CF;
#else
    outputBuffer[6] = bit_cast<uint16_t>(nextafter(-0.0h, 1.0h)); // CHECK-NEXT: 1
    outputBuffer[7] = bit_cast<uint16_t>(nextafter(0.0h, 1.0h)); // CHECK-NEXT: 1
    outputBuffer[8] = bit_cast<uint16_t>(nextafter(-0.0h, -1.0h)); // CHECK-NEXT: 8001
    outputBuffer[9] = bit_cast<uint16_t>(nextafter(0.0h, -1.0h)); // CHECK-NEXT: 8001
    outputBuffer[10] = bit_cast<uint16_t>(nextafter(1000.0h, 2000.0h)); // CHECK-NEXT: 63D1
    outputBuffer[11] = bit_cast<uint16_t>(nextafter(-1000.0h, -1.0h)); // CHECK-NEXT: E3CF
#endif

    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    write_double(nextafter(-0.0l, 1.0l), 12);
    // CHECK-NEXT: 1
    // CHECK-NEXT: 0
    write_double(nextafter(0.0l, 1.0l), 14);
    // CHECK-NEXT: 1
    // CHECK-NEXT: 80000000
    write_double(nextafter(-0.0l, -1.0l), 16);
    // CHECK-NEXT: 1
    // CHECK-NEXT: 80000000
    write_double(nextafter(0.0l, -1.0l), 18);
    // CHECK-NEXT: 1
    // CHECK-NEXT: 408F4000
    write_double(nextafter(1000.0l, 2000.0l), 20);
    // CHECK-NEXT: FFFFFFFF
    // CHECK-NEXT: C08F3FFF
    write_double(nextafter(-1000.0l, -1.0l), 22);
}
