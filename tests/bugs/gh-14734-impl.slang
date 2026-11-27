//TEST_IGNORE_FILE:

// The linked-in definitions of the extern struct types used by gh-14734.slang.
export struct Foo
{
    int v;
    int w;
};

export int getFooV(Foo f)
{
    return f.v;
}

export int getFooW(Foo f)
{
    return f.w;
}

export struct Bar
{
    int x;
    int y;
};

export int getBarSum(Bar b)
{
    return b.x + b.y;
}
