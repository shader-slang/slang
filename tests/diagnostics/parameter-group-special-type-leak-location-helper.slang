// Helper module for parameter-group-special-type-leak-location.slang.
//
// `Mixed` mixes ordinary data (float4) with resource types (Texture2D,
// SamplerState). When wrapped in a `ConstantBuffer<>` at the import site, the
// resource members "leak" out of the parameter group, triggering E31106 /
// E31107. Because the struct (and its field keys) are defined in this separate
// module, their source locations are not available at the point the warning is
// emitted in the importing module, which exercises the source-location
// fallback chains.

module "parameter-group-special-type-leak-location-helper";

public struct Mixed
{
    public float4 color;
    public Texture2D tex;
    public SamplerState samp;
}
