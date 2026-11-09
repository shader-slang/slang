//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Value
{
    int field;
    void set() { field = 1; }
//CHECK:         ^^^^^ attempting to assign to a const variable or immutable member
//CHECK:               ^ error E30011
//CHECK:               ^ left of '=' is not an l-value.
}

class Object
{
    let fixedValue : int = 1;
    property readOnly : int { get { return fixedValue; } }
    __init() {}

    void replace(Object other) { this = other; }
//CHECK:                         ^^^^ attempting to assign to a const variable or immutable member
//CHECK:                              ^ error E30011
//CHECK:                              ^ left of '=' is not an l-value.

    void changeFixed() { fixedValue = 2; }
//CHECK:                 ^^^^^^^^^^ attempting to assign to a const variable or immutable member
//CHECK:                            ^ error E30011
//CHECK:                            ^ left of '=' is not an l-value.
}

void test()
{
    let object = new Object();
    object.fixedValue = 3;
//CHECK:              ^ error E30011
//CHECK:              ^ left of '=' is not an l-value.
    object.readOnly = 4;
//CHECK:            ^ error E30011
//CHECK:            ^ left of '=' is not an l-value.
    object = new Object();
//CHECK:   ^ error E30011
//CHECK:   ^ left of '=' is not an l-value.
}
