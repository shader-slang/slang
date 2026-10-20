// The fence after a write to an element of a texture array names the same element as the write,
// so it must not make the emitter copy the array of texture handles into a local. Copying an
// unsized array that way is not valid MSL. (shader-slang/slang#13771)
//
// An unsized texture array is not a valid Metal kernel argument yet, so there is no metallib
// directive (see uniform-unsized-resource-array-unbound.slang).

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain -line-directive-mode none

RWTexture2D<float4> colors[];
RWTexture2D<uint> counters[];
RWTexture2D<float4> sizedColors[8];

// CHECK-LABEL: void paint
// CHECK-NOT: _S{{[0-9]+}}[] =
// CHECK: ->colors_{{[0-9]+}}[i_{{[0-9]+}}].write(
// CHECK-NEXT: _slang_textureFence({{.*}}->colors_{{[0-9]+}}[i_{{[0-9]+}}])
[noinline]
void paint(uint i, uint2 p)
{
    colors[i][p] = float4(1.0);
}

// CHECK-LABEL: void bump
// CHECK-NOT: _S{{[0-9]+}}[] =
// CHECK: ->counters_{{[0-9]+}}[i_{{[0-9]+}}]).atomic_fetch_add(
// CHECK-NEXT: _slang_textureFence({{.*}}->counters_{{[0-9]+}}[i_{{[0-9]+}}])
[noinline]
void bump(uint i, uint2 p)
{
    uint previous;
    InterlockedAdd(counters[i][p], 1, previous);
}

// CHECK-LABEL: void paintSized
// CHECK-NOT: array<texture2d
// CHECK: ->sizedColors_{{[0-9]+}}[i_{{[0-9]+}}].write(
// CHECK-NEXT: _slang_textureFence({{.*}}->sizedColors_{{[0-9]+}}[i_{{[0-9]+}}])
[noinline]
void paintSized(uint i, uint2 p)
{
    sizedColors[i][p] = float4(2.0);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    paint(id.z, id.xy);
    bump(id.z, id.xy);
    paintSized(id.z, id.xy);
}
