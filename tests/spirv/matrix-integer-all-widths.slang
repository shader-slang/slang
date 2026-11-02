// Integer matrices of every width are lowered to arrays of vectors on SPIR-V, which has no
// integer matrix types. Only `int` and `uint` matrices were lowered: a 16-bit integer matrix was
// emitted as an invalid `OpTypeMatrix` of integer vectors, and a conversion between two 16-bit
// integer matrix types crashed slangc ("Unhandled type passed to getIntTypeWidth").

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

// CHECK-NOT: {{error\[E[0-9]+\]}}
// CHECK-NOT: OpTypeMatrix %v{{[0-9]}}{{u?(short|char|long)}}
// CHECK: OpEntryPoint
// CHECK-NOT: OpTypeMatrix %v{{[0-9]}}{{u?(short|char|long)}}
// GLSL-NOT: {{error\[E[0-9]+\]}}
// GLSL-NOT: {{i16|u16|i64|u64|i8|u8}}mat
// GLSL: main

//TEST_INPUT:ubuffer(data=[3 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint seed = outputBuffer[0];

    // Arithmetic on a 16-bit integer matrix.
    matrix<uint16_t, 2, 3> a = matrix<uint16_t, 2, 3>(uint16_t(seed));
    a = a + a;
    a[1][2] = uint16_t(seed + 4);

    // Conversion between 16-bit integer matrix types, and the core-module path that uses it.
    matrix<int16_t, 2, 3> b = a;
    matrix<half, 2, 2> h = matrix<half, 2, 2>(half(seed));
    matrix<int16_t, 2, 2> hb = asint16(h);

    // 64-bit and 8-bit integer matrices.
    matrix<int64_t, 2, 2> c = matrix<int64_t, 2, 2>(int64_t(seed));
    c = c * int64_t(1000000000);
    matrix<uint8_t, 2, 2> d = matrix<uint8_t, 2, 2>(uint8_t(seed));
    d = d + d;

    // BUF: type: uint32_t
    // BUF-NEXT: 13
    outputBuffer[0] = uint(b[0][0]) + uint(b[1][2]);
    // half(3.0) is 0x4200.
    // BUF-NEXT: 16896
    outputBuffer[1] = uint(hb[1][1]);
    // BUF-NEXT: 3
    outputBuffer[2] = uint(c[1][0] / int64_t(1000000000));
    // BUF-NEXT: 6
    outputBuffer[3] = uint(d[1][1]);
}
