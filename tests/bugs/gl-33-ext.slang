// gl-33-ext.slang
//TEST_IGNORE_FILE:

struct A
{
    int state;
    [mutating] int next() { return state; }
};
