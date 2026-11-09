// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm,filecheck=DEPTH): -api vulkan -entry invalidDepth
// TEST:INTERPRET(hgvm,filecheck=STENCIL): -api vulkan -entry invalidStencil
// TEST:INTERPRET(hgvm,filecheck=DEPTH): -api d3d12 -entry invalidDepth
// TEST:INTERPRET(hgvm,filecheck=STENCIL): -api d3d12 -entry invalidStencil
// TEST:INTERPRET(hgvm,filecheck=MIPS): -api vulkan -entry invalidMipCount
// TEST:INTERPRET(hgvm,filecheck=MIPS): -api d3d12 -entry invalidMipCount
// DEPTH: result code = 1
// DEPTH: HGVM depth clear value must be finite and in [0, 1]
// STENCIL: result code = 1
// STENCIL: HGVM stencil clear value must be in [0, 255]
// MIPS: result code = 1
// MIPS: HGVM texture mip count exceeds its dimensions

import slang.runtime.resource.simple_texture;

[host]
void invalidDepth()
{
    allocateDepthTarget2D(8, 8).clear(-0.25);
}

[host]
void invalidStencil()
{
    allocateDepthTarget2D<TextureFormat.d32FloatS8Uint>(8, 8).clear<TextureAspect.StencilOnly>(
        256u);
}

[host]
void invalidMipCount()
{
    allocateTexture2D<float, TextureFormat.r32Float, 5>(8, 8).clear(0.0);
}
