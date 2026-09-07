// class-must-use-new.slang

// Test that a class can only be initialized with `new` (error 30066).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

class MyClass
{
    int x;
    __init() { x = 0; }
}

void test()
{
    MyClass obj = MyClass();
//CHECK:                 ^ error E30066
//CHECK:                 ^ a class can only be initialized by a `new` clause
}
