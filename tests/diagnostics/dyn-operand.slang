//TEST:SIMPLE(filecheck=CHECK): -no-codegen

interface IFoo
{
}
typealias Box = dyn IFoo;
typealias IntegerBox = dyn int;
typealias Reboxed = dyn Box;
typealias BoxConjunction = dyn IFoo & IFoo;

// CHECK-COUNT-3: error[E30321]
// CHECK-NOT: internal error
