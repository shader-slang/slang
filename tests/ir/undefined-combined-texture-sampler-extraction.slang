// Check that extracting the texture twice from one never-written combined texture-sampler yields
// one texture, not two independent ones, since an uninitialized value is one fixed value. The C++
// target makes the texture's identity visible through a target intrinsic that compares handles.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute -O0

__target_intrinsic(cpp, "(($0).texture == ($1).texture)")
bool isSameTexture(Texture2D<float4> a, Texture2D<float4> b);

// CHECK: Texture2D<Vector<float, 4> > [[TEX:[A-Za-z0-9_]+]] = {};
// CHECK-NOT: Texture2D<Vector<float, 4> > {{[A-Za-z0-9_]+}} = {};
// CHECK: (([[TEX]])).texture == (([[TEX]])).texture

[shader("compute")]
[numthreads(1, 1, 1)]
void main(RWStructuredBuffer<uint> outputBuffer)
{
    Sampler2D<float4> combined;
    outputBuffer[0] = isSameTexture(combined.__getTexture(), combined.__getTexture()) ? 1 : 0;
}
