//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test matching against severity and error codes

struct S {};

void test()
{
    if (1);
//CHECK ^ warning
//CHECK   ^ warning E20101
}
