// https://github.com/shader-slang/slang/issues/12764
//
// The redundant-forward-declaration handling is deliberately limited to
// non-generic structs: a generic struct is found by lookup through its
// enclosing `GenericDecl`, not the inner `StructDecl` the redeclaration check
// would hide, so hiding the inner decl would not suppress a cascade. Confirm a
// generic forward declaration does NOT take the new E30203 path and instead
// keeps the existing generic redeclaration diagnostics.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// A generic struct declared twice keeps the existing generic redeclaration
// diagnostic (E30200) — the new migration path is not taken.
//CHECK: E30200
//CHECK-NOT: E30203
//CHECK-NOT: redundant struct forward declaration
struct Foo<T>;

struct Foo<T>
{
    T value;
}
