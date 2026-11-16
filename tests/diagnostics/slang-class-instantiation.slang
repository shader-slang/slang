//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -no-codegen

// Regression test for #10305.
//
// Slang classes are reference types and cannot be laid out as shader parameters.
// Type layout must diagnose an unsupported class rather than crash.

class X
{
    int a;
};
X x;
//CHECK: class types are not supported in type layout
//CHECK: class type 'X' is not supported; use 'struct' instead
