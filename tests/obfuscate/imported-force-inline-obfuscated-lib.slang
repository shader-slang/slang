//TEST_IGNORE_FILE:

module "imported-force-inline-obfuscated-lib";

[__unsafeForceInlineEarly]
public int addOneForceInlineObf(int x)
{
    return x + 1;
}

public int tripleObf(int x)
{
    return x * 3;
}
