//TEST_IGNORE_FILE:

// Precompiled WITHOUT `-obfuscate` by `precompiled-plain-module.slang`, the way
// every standard module is built. No other test imports this module, so its
// binary never shadows a source import.

module "precompiled-plain-module-lib";

[__unsafeForceInlineEarly]
public int addOnePrecompiledPlain(int x)
{
    return x + 1;
}
