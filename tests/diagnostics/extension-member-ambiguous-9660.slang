//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660.
//
// When an `extension` declares a member with the same name and an overlapping signature as a
// member already declared on the extended type's primary declaration (or on a sibling
// `extension`), Slang does not currently define which declaration takes effect. The compiler
// emits warning 41037 to flag the conflict symmetrically, without stating which one is used.
//
// This file covers the POSITIVE cases (the compiler must warn). The exemptions that must NOT warn
// live in the sibling files extension-member-ambiguous-9660-exempt-*.slang so that each is checked
// on its own without ordering coupling.

// POSITIVE 1: a member function declared on both the primary `struct` and an `extension`, with the
// same name and the same signature, where the extension member is NOT satisfying an interface
// requirement and the extended type is concrete. This must warn (on both the extension member and
// the conflicting primary declaration via the note).
struct ConcreteFunc
{
    int memFunc() { return 0; }
//CHECK:^^^^^^^ see the conflicting declaration of 'memFunc' here
}

extension ConcreteFunc
{
    int memFunc() { return 1; }
//CHECK:^^^^^^^ member declared in more than one place
//CHECK:^^^^^^^ 'memFunc' is declared in more than one place (the extended type and/or its 'extension's); which declaration takes effect is currently undefined (see issue #9660)
}

// POSITIVE 2: a nested type declared with the same name on both the primary `struct` and an
// `extension`. Type members are compared by name alone, so this must warn.
struct ConcreteNested
{
    struct Inner { int a; }
//CHECK:   ^^^^^ see the conflicting declaration of 'Inner' here
}

extension ConcreteNested
{
    struct Inner { int b; }
//CHECK:   ^^^^^ member declared in more than one place
//CHECK:   ^^^^^ 'Inner' is declared in more than one place (the extended type and/or its 'extension's); which declaration takes effect is currently undefined (see issue #9660)
}
