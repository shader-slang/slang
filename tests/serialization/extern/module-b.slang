//TEST_IGNORE_FILE:

// module-b.slang

// This looks like a definition (and it is) but with [__extern] it's definition will be replaced at link time with a defintion
[__extern] struct Thing {};
[__extern] int foo(Thing thing);

int doSomething(Thing a, Thing b)
{
    return foo(a) + foo(b);
}


