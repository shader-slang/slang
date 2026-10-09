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
RWTexture2D<uint> t2DUint;
[format("rgba16f")]
RWTexture2D<float4> t2DHalf;
RWStructuredBuffer<float4> values;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // A single component goes through a `getElementPtr` on the texel. The texel that is
    // written back is the one read, with that component replaced.
    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t2D_{{[0-9]+}};
    // CHECK: float4 [[R:_S[0-9]+]] = surf2Dread<float4 >({{.*}}t2D_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    // CHECK: float4 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].w = 1.0f;
    // CHECK: surf2Dwrite<float4 >([[T]], [[H]], ({{.*}}).x * 16, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2D[tid.xy].w = 1.0f;

    // A multi-component swizzle goes through a `swizzledStore`, and its components are
    // assigned one at a time because CUDA vectors have no swizzle l-value.
    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t2D_{{[0-9]+}};
    // CHECK: float4 [[R:_S[0-9]+]] = surf2Dread<float4 >({{.*}}t2D_{{[0-9]+}}
    // CHECK: float4 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].x = [[XY:_S[0-9]+]].x;
    // CHECK-NEXT: [[T]].y = [[XY]].y;
    // CHECK: surf2Dwrite<float4 >([[T]], [[H]],
    t2D[tid.xy].xy = float2(2.0f, 3.0f);

    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t1D_{{[0-9]+}};
    // CHECK: float4 [[R:_S[0-9]+]] = surf1Dread<float4 >({{.*}}t1D_{{[0-9]+}}, ({{.*}}) * 16, SLANG_CUDA_BOUNDARY_MODE);
    // CHECK: float4 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].y = 4.0f;
    // CHECK: surf1Dwrite<float4 >([[T]], [[H]], ({{.*}}) * 16, SLANG_CUDA_BOUNDARY_MODE);
    t1D[tid.x].y = 4.0f;

    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t3D_{{[0-9]+}};
    // CHECK: float4 [[R:_S[0-9]+]] = surf3Dread<float4 >({{.*}}t3D_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE);
    // CHECK: float4 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].z = 5.0f;
    // CHECK: surf3Dwrite<float4 >([[T]], [[H]], ({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE);
    t3D[tid].z = 5.0f;

    // The array layer is the last coordinate of a layered surface access.
    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t2DArray_{{[0-9]+}};
    // CHECK: float4 [[R:_S[0-9]+]] = surf2DLayeredread<float4 >({{.*}}t2DArray_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE);
    // CHECK: float4 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].x = 6.0f;
    // CHECK: surf2DLayeredwrite<float4 >([[T]], [[H]], ({{.*}}).x * 16, ({{.*}}).y, ({{.*}}).z, SLANG_CUDA_BOUNDARY_MODE);
    t2DArray[tid].x = 6.0f;

    // The texel is accessed with the texture's own element type, so a `float2` texel is
    // 8 bytes wide rather than being widened to 4 components.
    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t2DFloat2_{{[0-9]+}};
    // CHECK: float2 [[R:_S[0-9]+]] = surf2Dread<float2 >({{.*}}t2DFloat2_{{[0-9]+}}, ({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    // CHECK: float2 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].y = 7.0f;
    // CHECK: surf2Dwrite<float2 >([[T]], [[H]], ({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2DFloat2[tid.xy].y = 7.0f;

    // A format conversion uses the `_convert` variants. The read is scaled by the size of
    // the backing `rgba16f` texel; the converting write is not byte addressed.
    // CHECK: CUsurfObject [[H:_S[0-9]+]] = {{.*}}t2DHalf_{{[0-9]+}};
    // CHECK: float4 [[R:_S[0-9]+]] = surf2Dread_convert<float4 >({{.*}}t2DHalf_{{[0-9]+}}, ({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    // CHECK: float4 [[T:_S[0-9]+]] = [[R]];
    // CHECK-NEXT: [[T]].w = 8.0f;
    // CHECK: surf2Dwrite_convert<float4 >([[T]], [[H]], ({{.*}}).x * 1, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2DHalf[tid.xy].w = 8.0f;

    // A dynamic component index cannot be a swizzle, so each component of the new texel is
    // selected on the index.
    // CHECK: float4 [[R:_S[0-9]+]] = surf2Dread<float4 >({{.*}}t2D_{{[0-9]+}}
    // CHECK: surf2Dwrite<float4 >(make_float4 (_slang_select({{.*}} == 0U, 9.0f,[[R]].x), _slang_select({{.*}} == 1U, 9.0f,[[R]].y), _slang_select({{.*}} == 2U, 9.0f,[[R]].z), _slang_select({{.*}} == 3U, 9.0f,[[R]].w)),
    t2D[tid.xy][tid.z] = 9.0f;

    // `.x` of a scalar texel addresses the texel itself, so it is a plain surface write.
    // CHECK-NOT: surf2Dread<uint>
    // CHECK: surf2Dwrite<uint>(16U, {{.*}}t2DUint_{{[0-9]+}}, ({{.*}}).x * 4, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2DUint[tid.xy].x = 16u;

    // A swizzle that assigns every component replaces the whole texel, so it needs no read:
    // `.wzyx = v` writes the texel `v.wzyx`.
    // CHECK: float4 [[V:_S[0-9]+]] = {{.*}}values_{{[0-9]+}}{{.*}};
    // CHECK-NOT: surf2Dread
    // CHECK: surf2Dwrite<float4 >({{.*}}[[V]].w, [[V]].z, [[V]].y, [[V]].x{{.*}}, {{.*}}t2D_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2D[tid.xy].wzyx = values[0];

    // The same holds for a two-component texel: `.yx = v` writes the texel `v.yx`.
    // CHECK: float2 [[V:_S[0-9]+]] = {{.*}};
    // CHECK-NOT: surf2Dread
    // CHECK: surf2Dwrite<float2 >({{.*}}[[V]].y, [[V]].x{{.*}}, {{.*}}t2DFloat2_{{[0-9]+}}, ({{.*}}).x * 8, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2DFloat2[tid.xy].yx = values[1].xy;

    // Each component of the texel takes the source component assigned to it: `.yzwx = v` writes
    // `v.wxyz`.
    // CHECK: float4 [[V:_S[0-9]+]] = {{.*}}values_{{[0-9]+}}{{.*}};
    // CHECK-NOT: surf2Dread
    // CHECK: surf2Dwrite<float4 >({{.*}}[[V]].w, [[V]].x, [[V]].y, [[V]].z{{.*}}, {{.*}}t2D_{{[0-9]+}}, ({{.*}}).x * 16, ({{.*}}).y, SLANG_CUDA_BOUNDARY_MODE);
    t2D[tid.xy].yzwx = values[2];
}

// No surface handle is ever subscripted like an array.
// NEG-NOT: ->t{{[0-9A-Za-z]+}}_{{[0-9]+}}[

// PTX: .entry
// PTX: suld
// PTX: sust
