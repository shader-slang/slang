//TEST_IGNORE_FILE:

[__unsafeForceInlineEarly]
int lexicalLeaf(int value)
{
    {
        int leafValue = value + 1;
        return leafValue;
    }
}

[ForceInline]
public int importedLexicalOuter(int value)
{
    {
        int nestedValue = lexicalLeaf(value);
        return nestedValue + 2;
    }
}
