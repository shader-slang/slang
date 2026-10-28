// `abs` of a 16-bit or 8-bit unsigned integer returns its input unchanged (#13983).
// The CPU and CUDA preludes define no 16-bit or 8-bit integer `abs`, so those targets are not run here.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature int16 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -render-feature int16 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -emit-spirv-via-glsl -entry computeMain -stage compute
// SPV-NOT: OpExtInst %{{(u|v[0-9]u)}}{{[a-z0-9]*}} %{{[A-Za-z0-9_]+}} SAbs

//TEST_INPUT:ubuffer(data=[0xFFFFFFFF 5 0xFFFFFFFB], stride=4):name=inBuf
RWStructuredBuffer<uint> inBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint16_t h = uint16_t(inBuf[0]);
    uint16_t hFive = uint16_t(inBuf[1]);
    outBuf[0] = uint(abs(h));
    vector<uint16_t, 2> hv = abs(vector<uint16_t, 2>(h, hFive));
    outBuf[1] = uint(hv.x);
    outBuf[2] = uint(hv.y);

    uint8_t b = uint8_t(inBuf[0]);
    outBuf[3] = uint(abs(b));
    outBuf[4] = uint(abs(vector<uint8_t, 2>(b, uint8_t(inBuf[1]))).x);

    // Signed controls: `abs(-5)` is still 5.
    outBuf[5] = uint(abs(int16_t(int(inBuf[2]))));
    outBuf[6] = uint(abs(int8_t(int(inBuf[2]))));
}

// CHECK: 65535
// CHECK-NEXT: 65535
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: 255
// CHECK-NEXT: 255
// CHECK-NEXT: {{^}}5{{$}}
// CHECK-NEXT: {{^}}5{{$}}
