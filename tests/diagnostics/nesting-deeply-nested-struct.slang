//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0,non-exhaustive):

// Test deeply nested structs - struct inside struct inside struct is valid.

struct A
{
    struct B
    {
        struct C
        {
            int x;
        }
        C c;
    }
    B b;
}
