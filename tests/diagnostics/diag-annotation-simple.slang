//DIAGNOSTIC_TEST(diag=CHECK):SIMPLE:

// Simple diagnostic annotation test

struct S
{
    int x;
};

void test()
{
    // This should trigger a warning about unused variable
    int unused;
}
//CHECK unused
