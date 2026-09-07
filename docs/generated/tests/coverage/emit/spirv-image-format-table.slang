//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins the SPIR-V image-format name that each `[format("...")]` attribute selects, for the 27 formats that no other test in the suite declares.
//META: intent=characterization
//META: covers=source/slang/slang-emit-spirv.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes `SPIRVEmitContext::getSpvImageFormat`, a flat `ImageFormat` ->
// `SpvImageFormat` translation table. It is pure data, so the only way it can be wrong
// is for a row to name the wrong SPIR-V constant, and the only way to notice is to
// compile one storage image per row and read back the format operand of its
// `OpTypeImage`. That is what this file does: one `RWTexture2D` per format, each read
// in the entry point so none is dead-stripped, and one `CHECK-DAG` per row asserting
// the emitted image type. The 27 formats here are the ones the rest of the suite never
// declares; the commonly used ones (rgba32f, r32f, rgba8, r32ui, r32i, ...) already
// have coverage elsewhere and are deliberately not repeated.
//
// The element type of each texture is chosen to match the format's component count and
// scalar class, because that is what fixes the sampled type in `OpTypeImage`
// (`%float` / `%int` / `%uint`); the format operand itself comes from the attribute.
// Note `rgb10_a2ui` emits SPIR-V's `Rgb10a2ui` spelling while `rgb10_a2` emits
// `Rgb10A2` -- the capitalisation genuinely differs between the two rows, and that is
// pinned rather than normalised. Several of these formats are outside SPIR-V's base
// set, so the module also declares `StorageImageExtendedFormats`.
// GLSL is pinned alongside SPIR-V because it runs the *other* half of the same
// mapping: `GLSLSourceEmitter::_emitGLSLImageFormatModifier` turns each attribute into
// a `layout(...)` qualifier, and all 27 formats have a GLSL spelling, so the two
// Khronos paths can be compared row by row. The element type also picks the image type
// prefix there (`image2D` / `iimage2D` / `uimage2D`), which the CHECKs pin next to each
// format so a row that lands on the wrong declaration fails.
//
// HLSL and Metal are not pinned: both drop the format entirely -- HLSL emits
// `RWTexture2D<float2>` and Metal `texture2d<float, access::read_write>` -- so there is
// no format token to assert on either target. WGSL is not pinned because its output is
// believed wrong: it has no spelling for 19 of these 27 formats and substitutes the
// float format `rgba32float` for all of them, including the integer ones, which makes
// the emitted `textureLoad` yield `f32` into an `i32`/`u32` variable. See
// `_meta/findings/wgsl-unsupported-int-image-format-falls-back-to-float.yaml`.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[format("rg32f")]
RWTexture2D<float2> t0;
[format("r11f_g11f_b10f")]
RWTexture2D<float3> t1;
[format("rgba16")]
RWTexture2D<float4> t2;
[format("rgb10_a2")]
RWTexture2D<float4> t3;
[format("rg16")]
RWTexture2D<float2> t4;
[format("r16")]
RWTexture2D<float> t5;
[format("r8")]
RWTexture2D<float> t6;
[format("rgba16_snorm")]
RWTexture2D<float4> t7;
[format("rgba8_snorm")]
RWTexture2D<float4> t8;
[format("rg16_snorm")]
RWTexture2D<float2> t9;
[format("rg8_snorm")]
RWTexture2D<float2> t10;
[format("r16_snorm")]
RWTexture2D<float> t11;
[format("r8_snorm")]
RWTexture2D<float> t12;
[format("rgba16i")]
RWTexture2D<int4> t13;
[format("rgba8i")]
RWTexture2D<int4> t14;
[format("rg32i")]
RWTexture2D<int2> t15;
[format("rg16i")]
RWTexture2D<int2> t16;
[format("rg8i")]
RWTexture2D<int2> t17;
[format("r16i")]
RWTexture2D<int> t18;
[format("r8i")]
RWTexture2D<int> t19;
[format("rgba16ui")]
RWTexture2D<uint4> t20;
[format("rgb10_a2ui")]
RWTexture2D<uint4> t21;
[format("rgba8ui")]
RWTexture2D<uint4> t22;
[format("rg32ui")]
RWTexture2D<uint2> t23;
[format("rg16ui")]
RWTexture2D<uint2> t24;
[format("rg8ui")]
RWTexture2D<uint2> t25;
[format("r16ui")]
RWTexture2D<uint> t26;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 c = tid.xy;
    float acc = 0;
    acc += float(t0[c].x);
    acc += float(t1[c].x);
    acc += float(t2[c].x);
    acc += float(t3[c].x);
    acc += float(t4[c].x);
    acc += float(t5[c]);
    acc += float(t6[c]);
    acc += float(t7[c].x);
    acc += float(t8[c].x);
    acc += float(t9[c].x);
    acc += float(t10[c].x);
    acc += float(t11[c]);
    acc += float(t12[c]);
    acc += float(t13[c].x);
    acc += float(t14[c].x);
    acc += float(t15[c].x);
    acc += float(t16[c].x);
    acc += float(t17[c].x);
    acc += float(t18[c]);
    acc += float(t19[c]);
    acc += float(t20[c].x);
    acc += float(t21[c].x);
    acc += float(t22[c].x);
    acc += float(t23[c].x);
    acc += float(t24[c].x);
    acc += float(t25[c].x);
    acc += float(t26[c]);
    outputBuffer[0] = acc;
}

// Formats outside SPIR-V's base storage-image set pull in this capability.
//SPV-DAG: OpCapability StorageImageExtendedFormats
// One row of the translation table per line; the sampled type is fixed by the
// texture's element type and the trailing token is the format operand under test.
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rg32f
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R11fG11fB10f
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba16
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgb10A2
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rg16
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R16
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R8
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba16Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rg16Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 Rg8Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R16Snorm
//SPV-DAG: OpTypeImage %float 2D 2 0 0 2 R8Snorm
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 Rgba16i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 Rgba8i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 Rg32i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 Rg16i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 Rg8i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 R16i
//SPV-DAG: OpTypeImage %int 2D 2 0 0 2 R8i
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rgba16ui
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rgb10a2ui
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rgba8ui
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rg32ui
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rg16ui
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 Rg8ui
//SPV-DAG: OpTypeImage %uint 2D 2 0 0 2 R16ui

// The GLSL half of the same table: one `layout(<format>)` qualifier per row, paired
// with the declaration it precedes so a format landing on the wrong image fails. The
// checks are ordered rather than -DAG, which also pins that declaration order survives
// emission. The element type picks the image-type prefix, so the integer rows read
// `iimage2D` / `uimage2D` rather than `image2D`.
//GLSL: layout(rg32f)
//GLSL: uniform image2D t0_0;
//GLSL: layout(r11f_g11f_b10f)
//GLSL: uniform image2D t1_0;
//GLSL: layout(rgba16)
//GLSL: uniform image2D t2_0;
//GLSL: layout(rgb10_a2)
//GLSL: uniform image2D t3_0;
//GLSL: layout(rg16)
//GLSL: uniform image2D t4_0;
//GLSL: layout(r16)
//GLSL: uniform image2D t5_0;
//GLSL: layout(r8)
//GLSL: uniform image2D t6_0;
//GLSL: layout(rgba16_snorm)
//GLSL: uniform image2D t7_0;
//GLSL: layout(rgba8_snorm)
//GLSL: uniform image2D t8_0;
//GLSL: layout(rg16_snorm)
//GLSL: uniform image2D t9_0;
//GLSL: layout(rg8_snorm)
//GLSL: uniform image2D t10_0;
//GLSL: layout(r16_snorm)
//GLSL: uniform image2D t11_0;
//GLSL: layout(r8_snorm)
//GLSL: uniform image2D t12_0;
//GLSL: layout(rgba16i)
//GLSL: uniform iimage2D t13_0;
//GLSL: layout(rgba8i)
//GLSL: uniform iimage2D t14_0;
//GLSL: layout(rg32i)
//GLSL: uniform iimage2D t15_0;
//GLSL: layout(rg16i)
//GLSL: uniform iimage2D t16_0;
//GLSL: layout(rg8i)
//GLSL: uniform iimage2D t17_0;
//GLSL: layout(r16i)
//GLSL: uniform iimage2D t18_0;
//GLSL: layout(r8i)
//GLSL: uniform iimage2D t19_0;
//GLSL: layout(rgba16ui)
//GLSL: uniform uimage2D t20_0;
//GLSL: layout(rgb10_a2ui)
//GLSL: uniform uimage2D t21_0;
//GLSL: layout(rgba8ui)
//GLSL: uniform uimage2D t22_0;
//GLSL: layout(rg32ui)
//GLSL: uniform uimage2D t23_0;
//GLSL: layout(rg16ui)
//GLSL: uniform uimage2D t24_0;
//GLSL: layout(rg8ui)
//GLSL: uniform uimage2D t25_0;
//GLSL: layout(r16ui)
//GLSL: uniform uimage2D t26_0;
