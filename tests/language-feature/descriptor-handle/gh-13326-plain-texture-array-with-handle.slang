// A plain `Texture2D` array keeps its type and is not split into texture and
// sampler arrays when the shader also has a `Texture2D.Handle` and a combined
// `Sampler2D`. The plain `Texture2D` type inst is shared by `t` and by the
// resource type of `h`, and the pass replaces every use of a type it lowers,
// so only combined texture types may be lowered to a `{texture, sampler}` struct.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=MTLLIB): -target metallib -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain

// MTL-NOT: Texture2D_0
// MTL-NOT: t_texture
// MTL-NOT: t_sampler
// MTL: void computeMain(
// MTL-SAME: array<texture2d<float, access::sample>, int(2)> t_1
// MTL-SAME: array<texture2d<float, access::sample>, int(2)> h_1
// MTL-SAME: array<texture2d<float, access::sample>, int(2)> c_texture_1
// MTL-SAME: array<sampler, int(2)> c_sampler_1
// MTL-NOT: Texture2D_0

// MTLLIB: define void @computeMain

// CPP-NOT: Texture2D_0
// CPP: struct GlobalParams_0
// CPP-NEXT: {
// CPP-NEXT: FixedArray<Texture2D<Vector<float, 4> >, 2> t_0;
// CPP-NEXT: FixedArray<Texture2D<Vector<float, 4> >, 2> h_0;
// CPP-NEXT: FixedArray<Sampler2D_0, 2> c_0;
// CPP-NOT: Texture2D_0

uniform Texture2D t[2];
uniform Texture2D.Handle h[2];
uniform Sampler2D c[2];
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x % 2;
    output[tid.x] = t[i].Load(int3(0, 0, 0)).x + h[i].Load(int3(0, 0, 0)).x + c[i].SampleLevel(float2(0, 0), 0).x;
}
