// class-inheritance-invalid.slang

// Test that a class cannot inherit from a struct (error 30814).
// This diagnostic emits identical summary and span text as one match, so pin its code and severity
// instead of adding a second message annotation that cannot bind.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct MyStruct
{
    int value;
}

class BadClass : MyStruct
//CHECK:         ^^^^^^^^ error E30814
{
    int extra;
}
