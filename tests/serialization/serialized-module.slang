//TEST_IGNORE_FILE:

// serialized-module.slang

struct Thing
{
    int a; 
    int b;
};

int foo(Thing thing) 
{ 
    return (thing.a + thing.b) - thing.b; 
}

