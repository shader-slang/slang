// struct-inheritance-invalid.slang

// Test that a struct cannot inherit from multiple base structs.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#pragma warning(disable:30816)

struct Base1
{
    int x;
}

struct Base2
{
    int y;
}

struct Derived : Base1, Base2
//CHECK:                ^^^^^ a struct type may only inherit from one other struct type, and that type must appear first in the list of bases
{
    int z;
}
