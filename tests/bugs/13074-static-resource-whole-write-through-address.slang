// Taking the address of the complete resource variable does not expose its identity when the only
// use stores a new resource value through that address. We treat this direct whole-value store as
// initialization and redirect it to the generated replacement local.

//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -o 13074-static-resource-whole-write-through-address.cpp
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -entry computeMain -shaderobj -output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000

//TEST_INPUT: Texture2D(size=4, content = one):name=inputTexture
Texture2D<float4> inputTexture;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=16):out,name=output
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture;

[noinline]
void initializeCachedTexture()
{
    *__getAddress(cachedTexture) = inputTexture;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    initializeCachedTexture();
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
