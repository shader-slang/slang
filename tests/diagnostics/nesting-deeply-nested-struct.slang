//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

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
