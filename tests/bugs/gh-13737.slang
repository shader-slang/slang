//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=NOSAMPLER): -target hlsl -entry hlslMain -stage compute
//TEST:SIMPLE(filecheck=DXIL): -target dxil-assembly -entry hlslMain -stage compute -profile cs_6_0
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry portableMain -stage compute
//TEST:SIMPLE(filecheck=NOSAMPLER): -target wgsl -entry portableMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry portableMain -stage compute
//TEST:SIMPLE(filecheck=NOSAMPLER): -target metal -entry portableMain -stage compute

// shader-slang/slang#13737 and #10522: texture-only operations (`Load`, the subscript and
// `GetDimensions`) on a combined texture-sampler must use only the texture. HLSL, Metal and WGSL
// have no combined texture-sampler type, so the compiler splits `Sampler2D` and friends into a
// texture and a sampler, and every call on the combined value receives both. Texture-only
// operations must ignore the injected sampler operand.
//
// The shader below never samples, so no sampler may be referenced as a call argument or index.

Sampler2D<float4> u_tex;
Sampler2DArray<float4> u_texArray;
Sampler2DMS<float4> u_texMS;
Sampler2DShadow u_shadow;
RWStructuredBuffer<float4> outBuf;

// NOSAMPLER-NOT: _sampler_{{[0-9]+}}{{[]),]}}

// Operations available on every target that splits combined texture-samplers.
float4 portableOps(int2 p)
{
    uint w, h, e, l, s;
    float fw, fh;
    u_tex.GetDimensions(w, h);
    float4 r = float4(w, h, 0, 0);
    u_texArray.GetDimensions(w, h, e);
    r += float4(w, h, e, 0);
    u_texArray.GetDimensions(0, w, h, e, l);
    r += float4(w, h, e, l);
    u_texMS.GetDimensions(w, h, s);
    r += float4(w, h, s, 0);
    u_shadow.GetDimensions(fw, fh);
    r += float4(fw, fh, 0, 0);
    r += u_tex.Load(int3(p, 0));
    r += u_tex[uint2(p)];
    r += u_texArray.Load(int4(p, 1, 0));
    r += u_texArray[uint3(p, 1)];
    r += u_shadow.Load(int3(p, 0));
    r += u_shadow[uint2(p)];
    r += u_texMS.Load(p, 1);
    r += u_texMS[uint2(p)];
    r += u_texMS[uint2(p), 3];
    return r;
}

// HLSL: u_tex_texture_{{[0-9]+}}.GetDimensions(w_{{[0-9]+}}, h_{{[0-9]+}});
// HLSL: u_texArray_texture_{{[0-9]+}}.GetDimensions(w_{{[0-9]+}}, h_{{[0-9]+}}, e_{{[0-9]+}});
// HLSL: u_texArray_texture_{{[0-9]+}}.GetDimensions(0U, w_{{[0-9]+}}, h_{{[0-9]+}}, e_{{[0-9]+}}, l_{{[0-9]+}});
// HLSL: u_texMS_texture_{{[0-9]+}}.GetDimensions(w_{{[0-9]+}}, h_{{[0-9]+}}, s_{{[0-9]+}});
// HLSL: u_shadow_texture_{{[0-9]+}}.GetDimensions(fw_{{[0-9]+}}, fh_{{[0-9]+}});
// HLSL: int3 [[LOC:_S[0-9]+]] = int3(p_{{[0-9]+}}, int(0));
// HLSL: uint2 [[IDX:_S[0-9]+]] = uint2(p_{{[0-9]+}});
// HLSL: int3 [[SUB:_S[0-9]+]] = int3(uint3([[IDX]], 0U));
// HLSL: int2 [[MSSUB:_S[0-9]+]] = int2([[IDX]]);
// HLSL: u_tex_texture_{{[0-9]+}}.Load([[LOC]])
// HLSL-SAME: u_tex_texture_{{[0-9]+}}.Load([[SUB]])
// HLSL-SAME: u_texArray_texture_{{[0-9]+}}.Load(int4(p_{{[0-9]+}}, int(1), int(0)))
// HLSL-SAME: u_texArray_texture_{{[0-9]+}}.Load(int4(uint4(uint3([[IDX]], 1U), 0U)))
// HLSL-SAME: u_shadow_texture_{{[0-9]+}}.Load([[LOC]])
// HLSL-SAME: u_shadow_texture_{{[0-9]+}}.Load([[SUB]])
// HLSL-SAME: u_texMS_texture_{{[0-9]+}}.Load(p_{{[0-9]+}}, int(1))
// HLSL-SAME: u_texMS_texture_{{[0-9]+}}.Load([[MSSUB]], int(0))
// HLSL-SAME: u_texMS_texture_{{[0-9]+}}.Load([[MSSUB]], int(3))
// HLSL: int2 [[OFF:_S[0-9]+]] = int2(int(1), int(1));
// HLSL: u_texMS_texture_{{[0-9]+}}.Load(int3(int2(id_{{[0-9]+}}.xy), int(2)).xy, int(2))
// HLSL-SAME: u_tex_texture_{{[0-9]+}}.Load(int3(int2(id_{{[0-9]+}}.xy), int(0)), [[OFF]])
// HLSL: u_tex_texture_{{[0-9]+}}.Load(int3(int2(id_{{[0-9]+}}.xy), int(0)), [[OFF]], status_{{[0-9]+}})
// HLSL: u_texMS_texture_{{[0-9]+}}.Load(int2(id_{{[0-9]+}}.xy), int(1), [[OFF]])
// HLSL: u_texMS_texture_{{[0-9]+}}.Load(int2(id_{{[0-9]+}}.xy), int(1), [[OFF]], msStatus_{{[0-9]+}})

// DXIL: call %dx.types.Dimensions @dx.op.getDimensions
// DXIL: call %dx.types.ResRet.f32 @dx.op.textureLoad

// WGSL: textureDimensions(
// WGSL: textureLoad((u_tex_texture_{{[0-9]+}})
// WGSL: textureLoad((u_texArray_texture_{{[0-9]+}})
// WGSL: textureLoad((u_texMS_texture_{{[0-9]+}})

// METAL: get_width(
// METAL: u_tex_texture_{{[0-9]+}}).read(
// METAL: u_texArray_texture_{{[0-9]+}}).read(
// METAL: u_texMS_texture_{{[0-9]+}}).read(

[shader("compute")]
[numthreads(1, 1, 1)]
void portableMain(uint3 id: SV_DispatchThreadID)
{
    outBuf[0] = portableOps(int2(id.xy));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void hlslMain(uint3 id: SV_DispatchThreadID)
{
    uint status;
    float4 r = portableOps(int2(id.xy));
    r += u_texMS.Load(int3(int2(id.xy), 2));
    r += u_tex.Load(int3(int2(id.xy), 0), int2(1, 1));
    r += u_tex.Load(int3(int2(id.xy), 0), int2(1, 1), status);
    uint msStatus;
    r += u_texMS.Load(int2(id.xy), 1, int2(1, 1));
    r += u_texMS.Load(int2(id.xy), 1, int2(1, 1), msStatus);
    outBuf[0] = r + float4(status) + float4(msStatus);
}
