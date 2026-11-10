//TEST_IGNORE_FILE:

// Helper for import-precompiled-global-varyings.slang, which imports it as a precompiled module. It
// stays legacy Slang so that the module-scope varying itself is accepted.

in float4 gPrecompiledColor;

float4 precompiledHelper()
{
    return float4(1);
}
