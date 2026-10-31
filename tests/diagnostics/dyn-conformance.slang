//TEST:SIMPLE(filecheck=CHECK): -std legacy -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2025 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2026 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 202c -no-codegen

interface IFoo
{
    static int get();
}
typealias Box = dyn IFoo;
int constrained<T : IFoo>()
{
    return T.get();
}

// A box stores a conforming payload; the box itself has no static requirement implementation.
int invalidConformance()
{
    return constrained<Box>();
}

// CHECK-NOT: internal error
// CHECK: error[E38029]
// CHECK-NOT: internal error
// CHECK: type argument 'dyn IFoo' does not conform to the required interface 'IFoo'
// CHECK-NOT: internal error
