// render-test generates every mip level of a `content=gradient` texture on the host, and a size-1
// level stores 0 in red, green and blue rather than converting 0 / 0 (NaN) to an integer, which is
// undefined behavior. x86-64 also gives 0 for that NaN in practice, so only a UBSan build can tell
// the two apart. The CPU lane makes the GPU-less sanitizer runner generate the float and the
// integer data. The CPU backend reads mip 0 for any `Load` mip coordinate, so only the GPU lanes
// read the 1x1 level back.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA32Float):name tFloat
Texture2D<float4> tFloat;
//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA32Uint):name tUint
Texture2D<uint4> tUint;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Mip 2, the 1x1 level of the size-4 chain (4, 2, 1), of the float texture and of the uint texture.
// CHECK: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000
// Texel (3, 3) of mip 0 of the uint texture, where the integer gradient is 1 in red and green.
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.000000
// CHECK-NEXT: 1.000000

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 lastMipFloat = tFloat.Load(int3(0, 0, 2));
    uint4 lastMipUint = tUint.Load(int3(0, 0, 2));
    uint4 mip0Texel33 = tUint.Load(int3(3, 3, 0));
    outputBuffer[0] = lastMipFloat.x;
    outputBuffer[1] = lastMipFloat.y;
    outputBuffer[2] = lastMipFloat.z;
    outputBuffer[3] = lastMipFloat.w;
    outputBuffer[4] = float(lastMipUint.x);
    outputBuffer[5] = float(lastMipUint.y);
    outputBuffer[6] = float(lastMipUint.z);
    outputBuffer[7] = float(lastMipUint.w);
    outputBuffer[8] = float(mip0Texel33.x);
    outputBuffer[9] = float(mip0Texel33.y);
    outputBuffer[10] = float(mip0Texel33.z);
    outputBuffer[11] = float(mip0Texel33.w);
}
