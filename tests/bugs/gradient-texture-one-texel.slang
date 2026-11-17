// render-test builds a `gradient` texture as x / (size - 1), and a texture with the full mip chain
// ends in a one-texel level, where size - 1 is 0 and the result is 0 / 0 = NaN. Converting NaN to
// unsigned char is undefined behavior that UBSan reports, so the nightly sanitizer job failed on
// any CPU test that used a gradient texture. A one-texel gradient must have 0 in its color
// channels. The `float` format and the `uint` format take different paths in render-test, and
// both are covered here.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT: Texture2D(size=1, content=gradient, format=RGBA32Float):name tf
Texture2D<float4> tf;
//TEST_INPUT: Texture2D(size=1, content=gradient, format=RGBA32Uint):name tu
Texture2D<uint4> tu;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// The float texel is (0, 0, 0, 1) and the uint texel is (0, 0, 0, 1).
// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 1

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 f = tf.Load(int3(0, 0, 0));
    uint4 u = tu.Load(int3(0, 0, 0));
    outputBuffer[0] = uint(f.x);
    outputBuffer[1] = uint(f.y);
    outputBuffer[2] = uint(f.z);
    outputBuffer[3] = uint(f.w);
    outputBuffer[4] = u.x;
    outputBuffer[5] = u.y;
    outputBuffer[6] = u.z;
    outputBuffer[7] = u.w;
}
