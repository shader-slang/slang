// Issue #13140: interfaces declared in an imported user module are offered by the
// missing-constraint suggestion when they are visible here (`public`), and not when they are
// `internal` to that module, since naming one in a constraint would not compile.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

module "suggest-constraint-imports";

import "suggest-constraint-lib";

int publicImport<T>(T t)
{
    return t.publicThing();
//CHECK:     ^^^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^^^ 'publicThing' is not a member of 'T'.
//CHECK:     ^^^^^^^^^^^ consider adding a constraint such as 'where T : IPublicThing', since 'publicThing' is declared by that interface
}

int publicGenericImport<T>(T t)
{
    return t.publicItem();
//CHECK:     ^^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^^ 'publicItem' is not a member of 'T'.
//CHECK:     ^^^^^^^^^^ consider constraining 'T' to interface 'IPublicBox', since 'publicItem' is declared by that interface
}

int internalImport<T>(T t)
{
    return t.internalThing();
//CHECK:     ^^^^^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^^^^^ 'internalThing' is not a member of 'T'.
}
