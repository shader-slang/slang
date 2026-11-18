// `bit_cast` between a `DescriptorHandle<T>` and its bits on the C++ and CUDA targets, where a
// handle is the native resource and therefore plain data (shader-slang/slang#13633). The bits go
// through a buffer and back, as a bare handle and as a handle nested in a struct, and the rebuilt
// handles must still read the texture.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

//TEST_INPUT: Texture2D(size=4, content = one):name=tex
uniform Texture2D.Handle tex;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=bits
RWStructuredBuffer<uint> bits;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct HandleAndIndex
{
    Texture2D.Handle texture;
    uint2 index;
}

[noinline]
void storeBits(uint2 v)
{
    bits[0] = v.x;
    bits[1] = v.y;
}

[noinline]
uint4 loadBits(uint2 index)
{
    return uint4(bits[0], bits[1], index);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    storeBits(bit_cast<uint2>(tex));
    uint4 v = loadBits(uint2(1, 2));

    let bare = bit_cast<Texture2D.Handle>(v.xy);
    let nested = bit_cast<HandleAndIndex>(v);

    // CHECK: type: float
    // CHECK-NEXT: 1.0
    outputBuffer[0] = bare.Load(int3(0)).x;
    // CHECK-NEXT: 1.0
    outputBuffer[1] = nested.texture.Load(int3(0)).x;
    // CHECK-NEXT: 3.0
    outputBuffer[2] = float(nested.index.x + nested.index.y);
    // CHECK-NEXT: 1.0
    outputBuffer[3] = float(bit_cast<uint4>(nested).x == v.x && bit_cast<uint2>(bare).y == v.y);
}

// CUDA-DAG: slang_bit_cast<CUtexObject>(
// CUDA-DAG: void computeMain
