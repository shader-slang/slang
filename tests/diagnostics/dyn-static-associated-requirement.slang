//TEST:SIMPLE(filecheck=CHECK): -std legacy -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2025 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2026 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 202c -no-codegen

// Both paths in #12430 must be diagnosed before IR generation: a static requirement reference
// through `IV` and explicit specialization of a constrained generic function with a box type.
interface IV
{
    associatedtype Differential;
    static Differential dzero();
}

T.Differential makeZero<T : IV>()
{
    return T.dzero();
}

void direct()
{
    let value = IV.dzero();
}
void throughGeneric()
{
    let value = makeZero<dyn IV>();
}

// CHECK-NOT: internal error
// CHECK: error[E30323]
// CHECK-NOT: internal error
// CHECK: error[E38029]
// CHECK-NOT: internal error
