//TEST:SIMPLE(filecheck=CHECK): -std 202c -no-codegen

interface IFoo {}
typealias Alias = IFoo;

void rejected(dyn IFoo value)
{
    IFoo();
    IFoo(value);
    Alias();
    Alias(value);
}

//CHECK-NOT: internal error
//CHECK: error[E30324]
//CHECK-SAME: write 'dyn
//CHECK-NOT: internal error
//CHECK: error[E30324]
//CHECK-SAME: write 'dyn
//CHECK-NOT: internal error
//CHECK: error[E30324]
//CHECK-SAME: write 'dyn
//CHECK-NOT: internal error
//CHECK: error[E30324]
//CHECK-SAME: write 'dyn
//CHECK-NOT: internal error
