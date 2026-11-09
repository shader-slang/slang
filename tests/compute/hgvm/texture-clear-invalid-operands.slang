// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:INTERPRET(hgvm,filecheck=COLOR): -api vulkan -entry colorAspect
//TEST:INTERPRET(hgvm,filecheck=STENCIL): -api vulkan -entry missingAspect
//TEST:INTERPRET(hgvm,filecheck=DEPTH): -api vulkan -entry missingStencil
//TEST:INTERPRET(hgvm,filecheck=BOUNDS): -api vulkan -entry wrongBounds
//TEST:INTERPRET(hgvm,filecheck=TYPE): -api vulkan -entry writableDepth
//COLOR: result code = 1
//COLOR: color clear requires All aspects and matching 32-bit scalar/vector values
//STENCIL: result code = 1
//STENCIL: stencil-only clear requires a stencil format and uint value
//DEPTH: result code = 1
//DEPTH: depth clear requires float depth and, when selected, uint stencil
//BOUNDS: result code = 1
//BOUNDS: clear subresource bounds must be uint values
//TYPE: result code = 1
//TYPE: depth/stencil storage requires a shader read-only texture type

[host]
void colorAspect(Texture2D<float> texture)
{
    hgbc_asm { OpClearTexture $$Texture2D<float> $texture DepthOnly $0u $1u $0u $1u $0.5; };
}

[host]
void missingAspect(Texture2D<float, 0, 44> texture)
{
    hgbc_asm { OpClearTexture $$Texture2D<float, 0, 44> $texture StencilOnly $0u $1u $0u $1u $7u; };
}

[host]
void missingStencil(Texture2D<float, 0, 46> texture)
{
    hgbc_asm { OpClearTexture $$Texture2D<float, 0, 46> $texture All $0u $1u $0u $1u $0.5; };
}

[host]
void wrongBounds(Texture2D<float> texture)
{
    hgbc_asm { OpClearTexture $$Texture2D<float> $texture All $0.0 $1u $0u $1u $0.5; };
}

[host]
void writableDepth(RWTexture2D<float, 0, 44> texture)
{
    hgbc_asm { OpClearTexture $$RWTexture2D<float, 0, 44> $texture All $0u $1u $0u $1u $0.5; };
}
