// Check that extracting the texture twice from one never-written combined texture-sampler yields
// one texture, not two independent ones, since an uninitialized value is one fixed value. The C++
// target lowers the combined value to a structure, and makes the texture's identity visible
// through a target intrinsic that compares handles.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute -O0

__target_intrinsic(cpp, "(($0).texture == ($1).texture)")
bool isSameTexture(Texture2D<float4> a, Texture2D<float4> b);

// CHECK: Sampler2D_{{[0-9]+}} [[COMBINED:[A-Za-z0-9_]+]] = {};
// CHECK-NOT: = {};
// CHECK: Texture2D<Vector<float, 4> > [[TEX:[A-Za-z0-9_]+]] = [[COMBINED]].texture_0;
// CHECK: (([[TEX]])).texture == (([[TEX]])).texture

[shader("compute")]
[numthreads(1, 1, 1)]
void main(RWStructuredBuffer<uint> outputBuffer)
{
    Sampler2D<float4> combined;
    outputBuffer[0] = isSameTexture(combined.__getTexture(), combined.__getTexture()) ? 1 : 0;
}
