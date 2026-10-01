// A write to part of a texel of an `RWTexture` (e.g. `tex[i].w = v`) has no native
// form on CUDA, because a surface write always stores a whole texel. We lower it to a
// read-modify-write of the whole texel through `surf*read`/`surf*write`
// (shader-slang/slang#12627).

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NEG): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

RWTexture1D<float4> t1D;
RWTexture2D<float4> t2D;
RWTexture3D<float4> t3D;
RWTexture2DArray<float4> t2DArray;
RWTexture2D<float2> t2DFloat2;
[format("rgba16f")]
RWTexture2D<float4> t2DHalf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // A single component goes through a `getElementPtr` on the texel.
    // CHECK: surf2Dread<float4 >({{.*}}t2D_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE)
    // CHECK: .w = 1.0f;
    // CHECK: surf2Dwrite<float4 >({{.*}}).x * 16, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE)
    t2D[tid.xy].w = 1.0f;

    // A multi-component swizzle goes through a `swizzledStore`, and its components are
    // assigned one at a time because CUDA vectors have no swizzle l-value.
    // CHECK: surf2Dread<float4 >({{.*}}t2D_{{[0-9]+}}
    // CHECK: .x = [[XY:_S[0-9]+]].x;
    // CHECK-NEXT: .y = [[XY]].y;
    // CHECK: surf2Dwrite<float4 >(
    t2D[tid.xy].xy = float2(2.0f, 3.0f);

    // CHECK: surf1Dread<float4 >({{.*}}t1D_{{[0-9]+}}, ({{.*}}) * 16, SLANG_CUDA_BOUNDARY_MODE)
    // CHECK: .y = 4.0f;
    // CHECK: surf1Dwrite<float4 >({{.*}}) * 16, SLANG_CUDA_BOUNDARY_MODE)
    t1D[tid.x].y = 4.0f;

    // CHECK: surf3Dread<float4 >({{.*}}t3D_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE)
    // CHECK: .z = 5.0f;
    // CHECK: surf3Dwrite<float4 >({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE)
    t3D[tid].z = 5.0f;

    // The array layer is the last coordinate of a layered surface access.
    // CHECK: surf2DLayeredread<float4 >({{.*}}t2DArray_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE)
    // CHECK: .x = 6.0f;
    // CHECK: surf2DLayeredwrite<float4 >({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE)
    t2DArray[tid].x = 6.0f;

    // The texel is accessed with the texture's own element type, so a `float2` texel is
    // 8 bytes wide rather than being widened to 4 components.
    // CHECK: surf2Dread<float2 >({{.*}}t2DFloat2_{{[0-9]+}}, ({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE)
    // CHECK: .y = 7.0f;
    // CHECK: surf2Dwrite<float2 >({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE)
    t2DFloat2[tid.xy].y = 7.0f;

    // A format conversion uses the `_convert` variants. The read is scaled by the size of
    // the backing `rgba16f` texel; the converting write is not byte addressed.
    // CHECK: surf2Dread_convert<float4 >({{.*}}t2DHalf_{{[0-9]+}}, ({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE)
    // CHECK: .w = 8.0f;
    // CHECK: surf2Dwrite_convert<float4 >({{.*}}).x * 1, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE)
    t2DHalf[tid.xy].w = 8.0f;

    // A dynamic component index cannot be a swizzle, so each component of the new texel is
    // selected on the index.
    // CHECK: surf2Dread<float4 >({{.*}}t2D_{{[0-9]+}}
    // CHECK: surf2Dwrite<float4 >(make_float4 (_slang_select({{.*}} == 0U, 9.0f,{{.*}}.x), _slang_select({{.*}} == 1U, 9.0f,{{.*}}.y), _slang_select({{.*}} == 2U, 9.0f,{{.*}}.z), _slang_select({{.*}} == 3U, 9.0f,{{.*}}.w)),
    t2D[tid.xy][tid.z] = 9.0f;
}

// No surface handle is ever subscripted like an array.
// NEG-NOT: ->t{{[0-9A-Za-z]+}}_{{[0-9]+}}[

// PTX: .entry
// PTX: suld
// PTX: sust
