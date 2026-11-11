// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:INTERPRET(hgvm,filecheck=RESIZE): -api vulkan -entry wrongDimensions
//TEST:INTERPRET(hgvm,filecheck=BOOL): -api vulkan -entry wrongAcquireResult
//TEST:INTERPRET(hgvm,filecheck=FORMAT): -api vulkan -entry wrongAcquireFormat
//TEST:INTERPRET(hgvm,filecheck=FORMAT): -api vulkan -entry missingAddress
//TEST:INTERPRET(hgvm,filecheck=PRESENT): -api vulkan -entry wrongPresentResult
//TEST:INTERPRET(hgvm,filecheck=PRESENT): -api vulkan -entry wrongWaitResult
//TEST:INTERPRET(hgvm,filecheck=PRESENT): -api vulkan -entry wrongWaitSurface
//TEST:INTERPRET(hgvm,filecheck=DIMENSIONS): -api vulkan -entry wrongDimensionResult
//TEST:INTERPRET(hgvm,filecheck=DIMENSIONS): -api vulkan -entry wrongDimensionSurface
//RESIZE: result code = 1
//RESIZE: requires a bool result, Surface operand, and uint dimensions
//BOOL: result code = 1
//BOOL: requires a bool result
//FORMAT: result code = 1
//FORMAT: result texture format must match the Surface operand
//PRESENT: result code = 1
//PRESENT: requires a bool result and Surface operand
//DIMENSIONS: result code = 1
//DIMENSIONS: requires a uint2 result and Surface operand

[host]
void wrongDimensions(Surface<43> surface)
{
    bool result = hgbc_asm {
        result : $$bool = OpResizeSurface $surface $1.0 $2u;
    };
    print(result);
}

[host]
void wrongAcquireResult(Surface<43> surface)
{
    RenderTarget2D<float4, 43> texture;
    uint result = hgbc_asm {
        result : $$uint = OpTryAcquireSurfaceTexture $surface &texture;
    };
    print(result);
}

[host]
void wrongAcquireFormat(Surface<43> surface)
{
    RenderTarget2D<float4, 10> texture;
    bool result = hgbc_asm {
        result : $$bool = OpTryAcquireSurfaceTexture $surface &texture;
    };
    print(result);
}

[host]
void missingAddress(Surface<43> surface, RenderTarget2D<float4, 43> texture)
{
    bool result = hgbc_asm {
        result : $$bool = OpTryAcquireSurfaceTexture $surface $texture;
    };
    print(result);
}

[host]
void wrongPresentResult(Surface<43> surface)
{
    uint result = hgbc_asm {
        result : $$uint = OpTryPresentSurface $surface;
    };
    print(result);
}

[host]
void wrongWaitResult(Surface<43> surface)
{
    uint result = hgbc_asm {
        result : $$uint = OpWaitForSurface $surface;
    };
    print(result);
}

[host]
void wrongWaitSurface(uint surface)
{
    bool result = hgbc_asm {
        result : $$bool = OpWaitForSurface $surface;
    };
    print(result);
}

[host]
void wrongDimensionResult(Surface<43> surface)
{
    uint result = hgbc_asm {
        result : $$uint = OpGetSurfaceDimensions $surface;
    };
    print(result);
}

[host]
void wrongDimensionSurface(uint surface)
{
    uint2 result = hgbc_asm {
        result : $$uint2 = OpGetSurfaceDimensions $surface;
    };
    print(result.x);
}
