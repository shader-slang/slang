// The render-test `content=gradient` texture stores x / (size - 1) in red, y / (size - 1) in green and
// z / (size - 1) in blue at each mip level, as a value in [0, 1] for float formats and as 0 or 1 for
// integer formats. The last mip level of a full mip chain is 1x1, where size - 1 is 0, so the
// generator stores 0 in red, green and blue there rather than converting 0 / 0 (NaN) to an integer,
// which is undefined behavior. These loads pin that 1x1 value for a float and a uint texture, and
// the CPU lane makes the sanitizer CI runner, which has no GPU, generate both the float and the
// integer Gradient data. In a plain build the NaN conversion also gives 0 on x86-64, so the
// undefined behavior shows up only as a UBSan report.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA32Float):name tFloat
Texture2D<float4> tFloat;
//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA32Uint):name tUint
Texture2D<uint4> tUint;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// The 1x1 mip level of the float texture.
// CHECK: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// The 1x1 mip level of the uint texture.
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// Texel (3, 3) of mip level 0 of the uint texture, where x / (size - 1) and y / (size - 1) are 1.
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 f = tFloat.Load(int3(0, 0, 2));
    uint4 u = tUint.Load(int3(0, 0, 2));
    uint4 u0 = tUint.Load(int3(3, 3, 0));
    outputBuffer[0] = f.x;
    outputBuffer[1] = f.y;
    outputBuffer[2] = f.z;
    outputBuffer[3] = f.w;
    outputBuffer[4] = float(u.x);
    outputBuffer[5] = float(u.y);
    outputBuffer[6] = float(u.z);
    outputBuffer[7] = float(u.w);
    outputBuffer[8] = float(u0.x);
    outputBuffer[9] = float(u0.y);
    outputBuffer[10] = float(u0.z);
    outputBuffer[11] = float(u0.w);
}
