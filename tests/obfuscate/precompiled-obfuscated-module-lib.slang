//TEST_IGNORE_FILE:

// Precompiled WITH `-obfuscate` by `precompiled-obfuscated-module.slang`. No
// other test imports this module, so its binary never shadows a source import.

module "precompiled-obfuscated-module-lib";

[__unsafeForceInlineEarly]
public int addOnePrecompiledObf(int x)
{
    return x + 1;
}
