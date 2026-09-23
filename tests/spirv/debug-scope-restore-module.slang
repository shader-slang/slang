//TEST_IGNORE_FILE:

[__unsafeForceInlineEarly]
int leaf(int value)
{
    return value + 1;
}

[ForceInline]
public int importedOuter(int value)
{
    int nested = leaf(value);
    return nested + 2;
}
