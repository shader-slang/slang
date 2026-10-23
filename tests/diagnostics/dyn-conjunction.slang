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
typealias Both = IFoo & IBar;

// A conjunction remains valid as an alias or constraint, but cannot be boxed.
void constrained<T : Both>(T value)
{
}
void implicitBox(Both value)
{
}
void explicitBox(dyn (IFoo & IBar) value)
{
}
struct Holder<T>
{
}
Holder<Both> nestedBox;

// CHECK-COUNT-3: error[E30325]
// CHECK-NOT: internal error
