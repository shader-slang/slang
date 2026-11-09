// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

public struct Viewport
{
    public float originX = 0;
    public float originY = 0;
    public float extentX = 0;
    public float extentY = 0;
    public float minZ = 0;
    public float maxZ = 1;
}

public struct ScissorRect
{
    public uint minX = 0;
    public uint minY = 0;
    public uint maxX = 0;
    public uint maxY = 0;
}

/// Dynamic draw values. Scissor enable belongs to the generic rasterizer policy; the rectangle
/// is a draw value. Defaults cover the complete attachment extent, preserving existing contents.
public struct DrawParameters
{
    public bool useDefaultViewport = true;
    public bool useDefaultScissor = true;
    public Viewport viewport;
    public ScissorRect scissor;
    public uint stencilRef = 0;

    public This withViewport(Viewport value)
    {
        This result = this;
        result.useDefaultViewport = false;
        result.viewport = value;
        return result;
    }

    public This withScissor(ScissorRect value)
    {
        This result = this;
        result.useDefaultScissor = false;
        result.scissor = value;
        return result;
    }

    [host]
    public __GraphicsDrawParameters lower()
    {
        return hgbc_asm
        {
        result:
            $$__GraphicsDrawParameters =
                OpMakeGraphicsDrawParameters $useDefaultViewport $useDefaultScissor
                    $viewport.originX $viewport.originY $viewport.extentX $viewport
                        .extentY $viewport.minZ $viewport.maxZ $scissor.minX $scissor.minY $scissor
                        .maxX $scissor.maxY $stencilRef;
        };
    }
}
