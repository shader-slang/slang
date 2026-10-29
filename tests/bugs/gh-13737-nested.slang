//TEST:SIMPLE(filecheck=CBUF): -target hlsl -entry cbufMain -stage compute
//TEST:SIMPLE(filecheck=NESTED): -target hlsl -entry nestedMain -stage compute
//TEST:SIMPLE(filecheck=EP): -target hlsl -entry epMain -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry cbufMain -stage compute -profile cs_6_0

// shader-slang/slang#13737: `Load`, the subscript and `GetDimensions` on a combined
// texture-sampler nested in a constant buffer, a struct or an entry-point parameter forward to
// the extracted texture, even when the shader never samples. The texture keeps the binding that
// reflection reports for it: `C.b` is the second texture in `C` (`t1`), `cb.i.t` follows `C`
// (`t2`), and the entry-point parameter follows the globals (`t3`).

cbuffer C
{
    Sampler2D<float4> a;
    Sampler2D<float4> b;
}

struct Inner { Sampler2D<float4> t; }
struct Outer { Inner i; float x; }
ConstantBuffer<Outer> cb;

RWStructuredBuffer<float4> outBuf;

// CBUF: Texture2D<float4 > [[B:C_b_texture_[0-9]+]] : register(t1);
// CBUF: [[B]].GetDimensions(
// CBUF: [[B]].Load(int3(int2(
// CBUF-SAME: [[B]].Load(int3(uint3(
[shader("compute")]
[numthreads(1, 1, 1)]
void cbufMain(uint3 id: SV_DispatchThreadID)
{
    uint w, h;
    b.GetDimensions(w, h);
    outBuf[0] = b.Load(int3(int2(id.xy), 0)) + b[id.xy] + float4(w, h, 0, 0);
}

// NESTED: Texture2D<float4 > cb_i_t_texture_{{[0-9]+}} : register(t2);
// NESTED: cb_i_t_texture_{{[0-9]+}}.Load(
[shader("compute")]
[numthreads(1, 1, 1)]
void nestedMain(uint3 id: SV_DispatchThreadID)
{
    outBuf[0] = cb.i.t.Load(int3(int2(id.xy), 0)) + cb.x;
}

// EP: Texture2D<float4 > entryPointParams_t_texture_{{[0-9]+}} : register(t3);
// EP: entryPointParams_t_texture_{{[0-9]+}}.Load(
[shader("compute")]
[numthreads(1, 1, 1)]
void epMain(uint3 id: SV_DispatchThreadID, uniform Sampler2D<float4> t)
{
    outBuf[0] = t.Load(int3(int2(id.xy), 0));
}

// DXIL: @dx.op.textureLoad
