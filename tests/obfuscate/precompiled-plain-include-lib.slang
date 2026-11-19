//TEST_IGNORE_FILE:

// Precompiled WITHOUT `-obfuscate` by `precompiled-plain-include.slang`. No
// other test imports this module, so its binary never shadows a source import.

module "precompiled-plain-include-lib";

[__unsafeForceInlineEarly]
public int addOnePlainInclude(int x)
{
    return x + 1;
}

[__unsafeForceInlineEarly]
public int addTwoPlainInclude(int x)
{
    return x + 2;
}
