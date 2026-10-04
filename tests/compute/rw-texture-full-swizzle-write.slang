// A swizzled write through a texture subscript reads the texel first unless it assigns every
// component of the texel the target's image store writes. Metal, GLSL and SPIR-V image stores
// always write four components, so a swizzle that covers a two-component texel still reads the
// texel, and the backing format's other channels survive; a swizzle that covers a
// four-component texel writes it directly (shader-slang/slang#12627).

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

[format("rgba32f")]
RWTexture2D<float2> gNarrow;
RWTexture2D<float4> gWide;
RWStructuredBuffer<float4> values;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // GLSL: vec4 [[R:_S[0-9]+]] = imageLoad(gNarrow_{{[0-9]+}},
    // GLSL: vec4 [[T:_S[0-9]+]] = [[R]];
    // GLSL: [[T]].yx = {{.*}};
    // GLSL: imageStore(gNarrow_{{[0-9]+}},{{.*}},[[T]]);
    // METAL: float4 [[R:_S[0-9]+]] = {{.*}}gNarrow_{{[0-9]+}}.read(
    // METAL: float4 [[T:_S[0-9]+]] = [[R]];
    // METAL: [[T]].yx = {{.*}};
    // METAL: gNarrow_{{[0-9]+}}.write([[T]],
    // SPIRV: [[R:%[0-9]+]] = OpImageRead %v4float
    // SPIRV: [[T:%[0-9]+]] = OpVectorShuffle %v4float [[R]] {{%[0-9]+}} 5 4 2 3
    // SPIRV: OpImageWrite {{%[0-9]+}} {{%[0-9]+}} [[T]]
    gNarrow[tid.xy].yx = values[0].xy;

    // GLSL-NOT: imageLoad(gWide
    // GLSL: imageStore(gWide_{{[0-9]+}},{{.*}}.wxyz);
    // METAL-NOT: gWide_{{[0-9]+}}.read(
    // METAL: gWide_{{[0-9]+}}.write({{.*}}.wxyz,
    // SPIRV-NOT: OpImageRead
    // SPIRV: OpImageWrite
    gWide[tid.xy].yzwx = values[1];
}
