//TEST_IGNORE_FILE:

// module-library-address-space-specialization-reuse.slang

// A library without an entry point for precompiled-spirv-address-space-specialization.slang
// (#14004), so its precompiled SPIR-V defines `scale` once and nothing else named `scale`.

module "module-library-address-space-specialization-reuse";

public void scale(inout float dst, float factor)
{
    dst *= factor;
}
