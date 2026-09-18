//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660, value-kind conflict.
//
// A static constant declared with the same name on both the primary `struct` and an `extension`
// is a value-kind member. Values are compared by name alone, so this must warn (41037).

struct ConcreteValue
{
    static const int X = 0;
//CHECK:             ^ see the conflicting declaration of 'X' here
}

extension ConcreteValue
{
    static const int X = 1;
//CHECK:             ^ member declared in more than one place
//CHECK:             ^ 'X' is declared in more than one place (the extended type and/or its 'extension's); which declaration takes effect is currently undefined (see issue #9660)
}
