//TEST:SIMPLE(filecheck=W):-target spirv -stage compute -entry computeMain -emit-spirv-directly -DWRITE_ONLY
//TEST:SIMPLE(filecheck=RW):-target spirv -stage compute -entry computeMain -emit-spirv-directly

// The heap global that a `DescriptorHandle<WTexture2D>` indexes is an unsized
// array of write-only textures, so it gets `NonReadable` like any other
// write-only texture array. The read-write heap at the same binding must not.
// W: OpDecorate %__slang_resource_heap{{.*}} NonReadable
// RW-NOT: NonReadable
// RW: %__slang_resource_heap = OpVariable

#ifdef WRITE_ONLY
uniform WTexture2D<float4>.Handle h;
#else
uniform RWTexture2D<float4>.Handle h;
#endif

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    h.Store(int2(0, 0), float4(1));
}
