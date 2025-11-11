//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_3 -g1

// CHECK: OpEntryPoint

public struct PortalView
{
    public float3 viewOrigin;
}

public struct PortalViews
{
    public PortalView views[64];
    public uint viewBits;
    public Optional<PortalView> PortalForStencil(uint stencil)
    {
        // -g1 causes `this` to be preloaded into a local variable for
        // debugging.
        // when emitting code for spirv_1_3 where there is no OpCopyLogical,
        // we have to generate a loop to copy the `views` array from the constant buffer
        // into the local variable for debugging.
        // The loop must be generated before control flow legalization for SPIRV
        // otherwise we will generate invalid SPIRV.
        if ((viewBits & stencil) == 0)
            return none;
        return views[firstbithigh(stencil)];
    }
};

ConstantBuffer<PortalViews> CPortals;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1,1,1)]
void main()
{
    if (let portal = CPortals.PortalForStencil(0xff))
    {
        outputBuffer[0] = portal.viewOrigin.x;
    }
}