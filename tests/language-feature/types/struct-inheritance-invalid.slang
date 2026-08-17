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
//CHECK:                ^^^^^ error E30820
{
    int z;
}
