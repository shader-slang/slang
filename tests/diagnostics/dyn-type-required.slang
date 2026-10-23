//TEST:SIMPLE(filecheck=CHECK): -std 202c -no-codegen

interface IFoo
{
}
typealias Alias = IFoo;

// Aliases retain interfaces, so the diagnostic applies equally through an alias.
void direct(IFoo value)
{
}
void aliased(Alias value)
{
}
struct Field
{
    IFoo value;
}
struct Holder<T>
{
}
Holder<IFoo> genericArgument;
IFoo arrayValue[2];

// CHECK-COUNT-5: error[E30324]
// CHECK-NOT: internal error
