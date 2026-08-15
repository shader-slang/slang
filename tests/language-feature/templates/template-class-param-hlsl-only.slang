//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -no-codegen

// `class` is accepted as a type-parameter spelling only inside a template's
// parameter list. A Slang generic must not silently treat `class` as
// `typename`, so `struct S<class T>` in ordinary Slang code is an error rather
// than a generic type parameter named `T`.

struct S<class T>
{
    T value;
}
//CHECK: class is not allowed here
