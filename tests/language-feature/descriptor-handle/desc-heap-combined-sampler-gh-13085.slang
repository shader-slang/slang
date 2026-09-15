// Regression test for shader-slang/slang#13085: a combined texture-sampler (e.g. `Sampler3D`)
// requires two descriptor-heap indices -- one for the texture and a separate one for the sampler.
// A single-index `ResourceDescriptorHeap[i]` handle cannot supply both, so an implicit conversion
// from it to a combined texture-sampler is rejected with E41400, directing the user to the explicit
// `.Handle(uint2(resourceIndex, samplerIndex))` form. Both conversion paths -- the direct `_Texture`
// initializer and the `.Handle`/`DescriptorHandle<T>` initializer -- are covered.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -profile spirv_1_6 -capability spvDescriptorHeapEXT -emit-spirv-directly

[shader("fragment")]
float4 main(float3 coordinate : TEXCOORD0) : SV_Target
{
    // Site 1: conversion via the `_Texture` type's own single-index initializer.
    //CHECK: error[E41400]
    //CHECK: static assertion failed, a combined texture-sampler requires separate resource and sampler heap indices{{.*}}use an explicit '.Handle(uint2(resourceIndex, samplerIndex))' to specify both indices
    Sampler3D volume = ResourceDescriptorHeap[11];

    // Site 2: conversion via the `.Handle` (DescriptorHandle<T>) single-index initializer.
    //CHECK: error[E41400]
    //CHECK: static assertion failed, a combined texture-sampler requires separate resource and sampler heap indices{{.*}}use an explicit '.Handle(uint2(resourceIndex, samplerIndex))' to specify both indices
    Sampler3D.Handle handle = ResourceDescriptorHeap[12];
    Sampler3D volume2 = handle;

    return volume.SampleLevel(coordinate, 0.0) + volume2.SampleLevel(coordinate, 0.0);
}
