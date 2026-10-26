//TEST:SIMPLE(filecheck=CHECK): -std legacy -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2025 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 2026 -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -std 202c -no-codegen

interface IFoo
{
}
interface IBar
{
}
struct Foo : IFoo
{
}
typealias Box = dyn IFoo;
typealias Both = IFoo & IBar;

void check(Foo value)
{
    let first = value is dyn IFoo;
    let second = value as dyn IFoo;
    let third = value is Box;
    let fourth = value as Box;
    let fifth = value is Both;
    let sixth = value as Both;
    let seventh = value is (IFoo & IBar);
    let eighth = value as (IFoo & IBar);
    // A type-level box operand has no payload to open and does not conform to `IFoo`.
    let ninth = Box is IFoo;
    let tenth = Box is Foo;
    let eleventh = IFoo is Foo;
}

// CHECK-NOT: internal error
// CHECK: error[E30322]
// CHECK-NOT: internal error
// CHECK: error[E30322]
// CHECK-NOT: internal error
// CHECK: error[E30322]
// CHECK-NOT: internal error
// CHECK: error[E30322]
// CHECK-NOT: internal error
// CHECK: error[E30326]
// CHECK-NOT: internal error
// CHECK: error[E30326]
// CHECK-NOT: internal error
// CHECK: error[E30326]
// CHECK-NOT: internal error
// CHECK: error[E30326]
// CHECK-NOT: internal error
// CHECK: error[E30301]
// CHECK-NOT: internal error
// CHECK: error[E30304]
// CHECK-NOT: internal error
// CHECK: error[E30300]
// CHECK-NOT: internal error
