// A plain `Texture2D.Handle` must keep its plain texture type on targets where
// descriptor handles stay native (Metal, CPU), even when the shader also uses a
// combined `Sampler2D`. Only the combined type is lowered to a
// `{texture, sampler}` struct.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=MTLLIB): -target metallib -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain

// MTL-NOT: Texture2D_0
// MTL: void computeMain({{.*}} texture2d<float, access::sample> h_1 {{\[\[}}texture(0){{\]\]}}, texture2d<float, access::sample> c_texture_1 {{\[\[}}texture(1){{\]\]}}, sampler c_sampler_1 {{\[\[}}sampler(0){{\]\]}})
// MTL-NOT: Texture2D_0

// MTLLIB: define void @computeMain

// CPP-NOT: Texture2D_0
// CPP: struct GlobalParams_0
// CPP-NEXT: {
// CPP-NEXT: Texture2D<Vector<float, 4> > h_0;
// CPP-NEXT: Sampler2D_0 c_0;
// CPP-NOT: Texture2D_0

uniform Texture2D.Handle h;
uniform Sampler2D c;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = h.Load(int3(0, 0, 0)).x + c.SampleLevel(float2(0, 0), 0).x;
}
