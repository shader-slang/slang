// Precompiled by the COMPILE step of import-shader-export-entrypoint.slang.
module import_shader_export_entrypoint_module;

[shader("vertex")]
export float4 exportedVs(float3 p) : SV_Position
{
    return float4(p, 1.0);
}
