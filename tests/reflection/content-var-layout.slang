// Loaded by the contentVarLayoutReflection unit test.
//TEST_IGNORE_FILE:

// S packs to 16 bytes.
struct S
{
    float3 pos;
    float u;
};

interface IFoo
{
    float get();
};

StructuredBuffer<S> gBuf;
RWStructuredBuffer<S> gRWBuf;
AppendStructuredBuffer<S> gAppendBuf;
ConstantBuffer<S> gCB;
ParameterBlock<S> gPB;
TextureBuffer<S> gTB;
Texture2D gTex;
StructuredBuffer<IFoo> gExistentialBuf;

// A `float3` element: its tight uniform size (12 bytes) differs from its array stride under rules
// that round vector alignment up (e.g. std430 → stride 16), so it distinguishes a stride computed
// through the array-layout rules from one that just copies the element size.
StructuredBuffer<float3> gVec3Buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // Reference the parameters so they are retained in the layout.
    S a = gBuf[0];
    gRWBuf[0] = a;
    gAppendBuf.Append(a);
    S b = gCB;
    S c = gPB;
    S d = gTB;
    float4 e = gTex.Load(int3(0, 0, 0));
    float f = gExistentialBuf[0].get();
    float3 g = gVec3Buf[0];
}
