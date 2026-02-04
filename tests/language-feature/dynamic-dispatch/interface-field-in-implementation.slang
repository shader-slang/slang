//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

interface IFoo
{
    void foo();
}

interface IBar
{
    void bar();
}

struct BadImpl1 : IFoo
{
    // CHECK: ([[# @LINE+1]]): error 33083: cannot have interface-typed member 'inner' when conforming to interface 'IFoo'.
    IFoo inner;
    void foo() {}
};

struct BadImpl2 : IFoo
{
    // CHECK: ([[# @LINE+1]]): error 33083: cannot have interface-typed member 'inners' when conforming to interface 'IFoo'.
    IFoo[3] inners;
    void foo() {}
};

struct BadImpl3 : IFoo
{
    // CHECK: ([[# @LINE+1]]): error 33083: cannot have interface-typed member 'other' when conforming to interface 'IFoo'.
    IBar other;
    void foo() {}
};

[numthreads(1, 1, 1)]
void computeMain()
{
}
