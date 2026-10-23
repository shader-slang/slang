// Handles to non-combined resources other than `Texture2D` keep their native
// resource types on Metal and CPU when the shader also uses a combined
// `Sampler2D`: a writable texture, a 3D texture, and a handle read from a
// structured buffer.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain

// MTL-NOT: RWTexture2D_0
// MTL-NOT: Texture3D_0
// MTL-NOT: Texture2D_0
// MTL: void computeMain(
// MTL-SAME: texture2d<float, access::read_write> rw_1 {{\[\[}}texture({{[0-9]+}}){{\]\]}}
// MTL-SAME: texture3d<float, access::sample> v_1 {{\[\[}}texture({{[0-9]+}}){{\]\]}}
// MTL-SAME: texture2d<float, access::sample> device* hs_1 {{\[\[}}buffer({{[0-9]+}}){{\]\]}}
// MTL-NOT: RWTexture2D_0
// MTL-NOT: Texture3D_0
// MTL-NOT: Texture2D_0

// CPP-NOT: RWTexture2D_0
// CPP-NOT: Texture3D_0
// CPP-NOT: Texture2D_0
// CPP: struct GlobalParams_0
// CPP-NEXT: {
// CPP-NEXT: RWTexture2D<Vector<float, 4> > rw_0;
// CPP-NEXT: Texture3D<Vector<float, 4> > v_0;
// CPP-NEXT: StructuredBuffer<Texture2D<Vector<float, 4> >> hs_0;
// CPP-NEXT: Sampler2D_0 c_0;
// CPP-NOT: RWTexture2D_0
// CPP-NOT: Texture3D_0
// CPP-NOT: Texture2D_0

uniform RWTexture2D<float4>.Handle rw;
uniform Texture3D.Handle v;
uniform StructuredBuffer<Texture2D.Handle> hs;
uniform Sampler2D c;
RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    rw[uint2(0, 0)] = float4(1);
    output[tid.x] = v.Load(int4(0, 0, 0, 0)).x + hs[0].Load(int3(0, 0, 0)).x + c.SampleLevel(float2(0, 0), 0).x;
}
