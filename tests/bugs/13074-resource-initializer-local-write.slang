// `selectThroughLocal` writes only its own local variables. No code outside the call can observe
// those stores, so moving the call to the start of each entry point preserves behavior. This test
// also covers the uniform parameter `selectedTexture`. IR lowering packs that immutable parameter
// into a synthesized parameter group. The packing must not make the initializer appear to read
// mutable global storage.
//
// We use the C++ target because it can represent the resource-valued locals in
// `selectThroughLocal`. This test concerns the initializer analysis rather than a target's rules
// for local resource storage.
//TEST:SIMPLE: -target cpp -entry computeMain -stage compute -O0 -validate-ir -o 13074-resource-initializer-local-write.cpp
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj -output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000

//TEST_INPUT: Texture2D(size=4, content = zero):name=firstTexture
Texture2D<float4> firstTexture;

//TEST_INPUT: Texture2D(size=4, content = one):name=secondTexture
Texture2D<float4> secondTexture;

//TEST_INPUT: uniform(data=[1]):name=selectedTexture
uniform uint selectedTexture;

// The output starts with twos, while the selected texture contains ones. The expected result
// therefore proves both that the initializer selected the second texture and that the shader wrote
// the loaded value.
//TEST_INPUT: ubuffer(data=[2 2 2 2], stride=16):out,name=output
RWStructuredBuffer<float4> output;

[noinline]
Texture2D<float4> selectThroughLocal(
    Texture2D<float4> first,
    Texture2D<float4> second,
    uint index)
{
    Texture2D<float4> firstLocal = first;
    Texture2D<float4> secondLocal = second;
    Ptr<Texture2D<float4>> selectedLocal = index == 0 ? &firstLocal : &secondLocal;
    return *selectedLocal;
}

static Texture2D<float4> cachedTexture =
    selectThroughLocal(firstTexture, secondTexture, selectedTexture);

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
