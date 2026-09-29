// A plain `Texture2D` array must not be split into texture and sampler arrays
// when the shader also has a `Texture2D.Handle` and a combined `Sampler2D`.
// The plain `Texture2D` type is shared by `t` and by the handle's pointee, so
// lowering it as if it were combined also retyped `t` and left its layout
// inconsistent (an internal error in Debug builds).

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain

// MTL-NOT: E99997
// MTL-NOT: Texture2D_0
// MTL-NOT: t_texture
// MTL-NOT: t_sampler
// MTL: void computeMain({{.*}} array<texture2d<float, access::sample>, int(2)> t_1, array<texture2d<float, access::sample>, int(2)> h_1, array<texture2d<float, access::sample>, int(2)> c_texture_1, array<sampler, int(2)> c_sampler_1)
// MTL-NOT: Texture2D_0

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
