//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the image format slangc infers for a writable texture from its element type for the 8-bit, 64-bit and half element widths.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: when a writable texture carries no explicit `[format(...)]`
// attribute, `inferImageFormatFromTextureType` derives one from the element
// type's base type and vector width. The element types below select the arms of
// that switch that nothing else in the suite reaches (8-bit signed/unsigned,
// 64-bit signed/unsigned, and half at widths 1/2/4). CHECK pins the `layout(...)`
// qualifier GLSL emission writes for each declaration, which is the only
// user-visible evidence of what the inference chose.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

RWTexture2D<uint8_t> a0;
RWTexture2D<vector<uint8_t, 2> > a1;
RWTexture2D<vector<uint8_t, 4> > a2;
RWTexture2D<int8_t> b0;
RWTexture2D<vector<int8_t, 2> > b1;
RWTexture2D<vector<int8_t, 4> > b2;
RWTexture2D<int64_t> c0;
RWTexture2D<uint64_t> d0;
RWTexture2D<half> e0;
RWTexture2D<vector<half, 2> > e1;
RWTexture2D<vector<half, 4> > e2;

// CHECK: layout(r8ui)
// CHECK: uniform u8image2D a0_{{[0-9]+}};
// CHECK: layout(rg8ui)
// CHECK: uniform u8image2D a1_{{[0-9]+}};
// CHECK: layout(rgba8ui)
// CHECK: uniform u8image2D a2_{{[0-9]+}};
// CHECK: layout(r8i)
// CHECK: uniform i8image2D b0_{{[0-9]+}};
// CHECK: layout(rg8i)
// CHECK: uniform i8image2D b1_{{[0-9]+}};
// CHECK: layout(rgba8i)
// CHECK: uniform i8image2D b2_{{[0-9]+}};
// CHECK: layout(r64i)
// CHECK: uniform i64image2D c0_{{[0-9]+}};
// CHECK: layout(r64ui)
// CHECK: uniform u64image2D d0_{{[0-9]+}};
// CHECK: layout(r16f)
// CHECK: uniform image2D e0_{{[0-9]+}};
// CHECK: layout(rg16f)
// CHECK: uniform image2D e1_{{[0-9]+}};
// CHECK: layout(rgba16f)
// CHECK: uniform image2D e2_{{[0-9]+}};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    a0[tid.xy] = uint8_t(1);
    a1[tid.xy] = vector<uint8_t, 2>(uint8_t(1));
    a2[tid.xy] = vector<uint8_t, 4>(uint8_t(1));
    b0[tid.xy] = int8_t(1);
    b1[tid.xy] = vector<int8_t, 2>(int8_t(1));
    b2[tid.xy] = vector<int8_t, 4>(int8_t(1));
    c0[tid.xy] = 1;
    d0[tid.xy] = 1;
    e0[tid.xy] = half(1);
    e1[tid.xy] = vector<half, 2>(half(1));
    e2[tid.xy] = vector<half, 4>(half(1));
}
