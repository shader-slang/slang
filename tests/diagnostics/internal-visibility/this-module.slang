//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

module "this-module";

import "that-module";

void test()
{
    PublicStruct s;
    // Error: accessing internal member
    s.x = 1; // Error.
/*CHECK:
      ^ declaration not accessible
      ^ 'x' is not accessible from the current context.
*/
    s.y = 2; // OK.
    publicMethod(); // OK.
    publicFunc2();  // OK.
    Namespace.publicVar = 1; // OK.
    Namespace.publicVar2 = 1; // OK.
    // Error: accessing internal var
    Namespace.internalVar = 1; // error.
//CHECK:      ^^^^^^^^^^^ declaration not accessible
//CHECK:      ^^^^^^^^^^^ 'internalVar' is not accessible from the current context.
    // Error: using internal enum
    InternalEnum e; // Error.
/*CHECK:
    ^^^^^^^^^^^^ declaration not accessible
    ^^^^^^^^^^^^ 'InternalEnum' is not accessible from the current context.
*/
    // Error: using internal struct
    InternalStruct s1; // Error.
/*CHECK:
    ^^^^^^^^^^^^^^ declaration not accessible
    ^^^^^^^^^^^^^^ 'InternalStruct' is not accessible from the current context.
*/
    // Error: calling internal method
    internalMethod(); // Error.
/*CHECK:
    ^^^^^^^^^^^^^^ declaration not accessible
    ^^^^^^^^^^^^^^ 'internalMethod' is not accessible from the current context.
*/
}
