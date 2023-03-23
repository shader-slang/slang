//TEST_IGNORE_FILE:

// obfuscated-serialized-module.slang

struct Thing
{
    int a; 
    int b;
};

int foo(Thing thing) 
{ 
    return (thing.a + thing.b) - thing.b; 
}

