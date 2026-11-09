// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:INTERPRET(hgvm,filecheck=RESIZE): -api vulkan -entry wrongDimensions
//TEST:INTERPRET(hgvm,filecheck=BOOL): -api vulkan -entry wrongAcquireResult
//TEST:INTERPRET(hgvm,filecheck=FORMAT): -api vulkan -entry wrongAcquireFormat
//TEST:INTERPRET(hgvm,filecheck=FORMAT): -api vulkan -entry missingAddress
//TEST:INTERPRET(hgvm,filecheck=PRESENT): -api vulkan -entry wrongPresentResult
//RESIZE: result code = 1
//RESIZE: requires a bool result, Surface operand, and uint dimensions
//BOOL: result code = 1
//BOOL: requires a bool result
//FORMAT: result code = 1
//FORMAT: result texture format must match the Surface operand
//PRESENT: result code = 1
//PRESENT: requires a bool result and Surface operand

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
