// gl-33-ext.slang
//TEST_IGNORE_FILE:

public struct A
{
    public int state;
    [mutating] public int next() { return state; }
};
