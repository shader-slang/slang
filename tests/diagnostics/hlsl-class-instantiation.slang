//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-lang hlsl -target spirv -no-codegen

// Regression test for #10305.
//
// Instantiating an HLSL `class` type used to crash with a segfault
// (or an "unimplemented case in type layout" assertion in debug builds)
// because `_createTypeLayout` had no handler for `ClassDecl`.

class X { int a; };
/*CHECK:
      ^ class types are not supported for the current compilation target
      ^ class type 'X' is not supported; use 'struct' instead
*/
X x;
