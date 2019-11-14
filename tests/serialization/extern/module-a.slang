//TEST_IGNORE_FILE:

// module-a.slang

struct Thing
{
    int a; 
    int b;
};

Thing makeThing(int a, int b)
{
    Thing thing;
    thing.a = a;
    thing.b = b;
    return thing;
}

int foo(Thing thing) 
{ 
    return thing.a + thing.b * 2; 
}

