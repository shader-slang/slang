// Regression test for #11664: an `operator <op>` name is only meaningful when declaring an
// operator *function*. Using it for any other C-style declaration -- a variable, a parameter, a
// `typedef`, or a `property` -- must be diagnosed at the declaration with E20020, not accepted
// silently. A distinct operator is used per case so the parser's same-location duplicate-error
// filter cannot mask a missing diagnostic. The legal operator-function forms are checked in the
// companion test `operator-name-on-function.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Variable.
int operator+ = 10;
//CHECK: E20020

// Both positions of a multi-declarator variable list.
int operator- = 1, operator* = 2;
//CHECK: E20020
//CHECK: E20020

// Traditional-style function parameter.
void f(int operator/) {}
//CHECK: E20020

// Typedef alias name.
typedef int operator%;
//CHECK: E20020

// Property (traditional-syntax) name.
struct S
{
    property int operator& { get { return 0; } }
    //CHECK: E20020
}

// The legal operator-*function* forms that must still compile are covered in the companion test
// `operator-name-on-function.slang`.
