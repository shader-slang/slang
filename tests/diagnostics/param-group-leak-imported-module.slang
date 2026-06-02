//TEST_IGNORE_FILE:

// Module used by param-group-leak-imported.slang (regression for
// shader-slang/slang#11395). It provides a struct with resource members that,
// when used inside a ConstantBuffer in a consumer, causes resource members to
// "leak" out of the parameter group and triggers warnings E31106/E31107. The
// point of the test is that the struct (and therefore its field keys) is
// imported from a precompiled binary module, so the field keys carry no source
// location that resolves in the consuming compile.

module param_group_leak_imported;

public struct Material
{
    public Texture2D albedo;
    public SamplerState samp;
    public float4 tint;
}
