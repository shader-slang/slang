// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry mipStart
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry mipCount
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry layerStart
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api vulkan -entry layerCount
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12 -entry mipStart
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12 -entry mipCount
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12 -entry layerStart
// TEST:INTERPRET(hgvm,filecheck=CHECK): -api d3d12 -entry layerCount
// CHECK: result code = 1
// CHECK: HGVM texture clear subresource range is out of bounds

import slang.runtime.resource.simple_texture;

[host]
void clearRange(SubresourceRange range)
{
    let texture = allocateTexture2D<float, TextureFormat.r32Float>(8, 8);
    texture.clear(0.0, range);
}

[host]
void mipStart()
{
    clearRange(SubresourceRange(2, 0, 0, 1));
}
[host]
void mipCount()
{
    clearRange(SubresourceRange(1, 0xfffffffeu, 0, 1));
}
[host]
void layerStart()
{
    clearRange(SubresourceRange(0, 1, 2, 0));
}
[host]
void layerCount()
{
    clearRange(SubresourceRange(0, 1, 1, 0xfffffffeu));
}
