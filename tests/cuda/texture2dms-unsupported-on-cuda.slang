// #12633. Textures are unused: the gap is in type lowering, not any method.
// The two multisampled textures use distinct element types so they do not
// deduplicate into one another; each of the two paths below is checked
// independently. A non-multisampled negative control lives in the sibling
// test `texture2d-supported-on-cuda.slang`, which must still lower on CUDA.
//
// `non-exhaustive` is required: in machine-readable mode each E55215 also emits
// a `span` sub-record for its caret, so the compiler reports more diagnostic
// records than the two `E55215` lines annotated here.
//
// CUDA source emission exercises the pre-emit `checkUnsupportedInst` diagnostic
// without requiring the `nvrtc` downstream compiler on CPU CI runners.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry main -stage compute

Texture2DMS<float> directTexture;
//CHECK: E55215

struct Params { Texture2DMS<int> nestedTexture; }
ConstantBuffer<Params> params;
//CHECK: E55215

RWStructuredBuffer<uint> output;

// CPP: Texture2DMS

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = 1;
}
