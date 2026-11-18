// A `bit_cast` into a `DescriptorHandle<T>` that the compiler cannot build from bits reports an
// error at the cast instead of aborting with an internal error (shader-slang/slang#13633). Outside
// C++ and CUDA a handle is not plain data. On C++ and CUDA a handle nested in an aggregate is built
// from one integer leaf, which a 16-byte structured-buffer handle is not; a bare handle of the
// wrong size gets the usual size mismatch.

//DIAGNOSTIC_TEST:SIMPLE(diag=SPV):-target spirv -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CPP):-target cpp -entry cppMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=OTHER):-target hlsl -entry spirvMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=OTHER):-target metal -entry spirvMain -stage compute

uniform uint2 handleBits;
uniform uint4 wideBits;
RWStructuredBuffer<float4> outputBuffer;

struct BufferAndIndex
{
    StructuredBuffer<float4>.Handle buffer;
    uint4 index;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void spirvMain()
{
    let t = bit_cast<Texture2D.Handle>(handleBits);
//SPV:                                ^ error E41205
//SPV:                                ^ cannot bit_cast 'vector<uint,2>' to 'DescriptorHandle<Texture2D>'
//SPV:                                ^ DescriptorHandle constructor
//OTHER:                              ^ error E41205
//OTHER:                              ^ cannot bit_cast 'vector<uint,2>' to 'DescriptorHandle<Texture2D>'
//OTHER:                              ^ DescriptorHandle constructor
    outputBuffer[0] = t.Load(int3(0));
}

struct WideBits
{
    uint4 a;
    uint4 b;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cppMain()
{
    let s = bit_cast<BufferAndIndex>(WideBits(wideBits, wideBits));
//CPP:                              ^ error E41205
//CPP:                              ^ 'DescriptorHandle<StructuredBuffer<vector<float,4>>>' cannot be built from bits
//CPP:                              ^ DescriptorHandle constructor
    let b = bit_cast<StructuredBuffer<float4>.Handle>(handleBits);
//CPP:                                               ^ error E41202
//CPP:                                               ^ with size '8' casted into 'DescriptorHandle<StructuredBuffer<vector<float,4>>>' with size '16'
    outputBuffer[0] = s.buffer[s.index.x] + b[0];
}
