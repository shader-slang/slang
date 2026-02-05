//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test for issue #9835: An implementation of IFoo should NOT be allowed
// to contain IFoo or IFoo[] typed fields, as this creates infinite recursion
// in the existential representation.

interface IFoo
{
    void foo();
}

interface IBar
{
    void bar();
}

// Case 1: Direct self-reference - should error
struct BadImpl1 : IFoo
{
    // CHECK: ([[# @LINE+1]]): error 33083: cannot have interface-typed member 'inner' when conforming to interface 'IFoo'.
    IFoo inner;
    void foo() {}
};

// Case 2: Array of self-reference - should error
struct BadImpl2 : IFoo
{
    // CHECK: ([[# @LINE+1]]): error 33083: cannot have interface-typed member 'inners' when conforming to interface 'IFoo'.
    IFoo[3] inners;
    void foo() {}
};

// Case 3: Different interface - allowed (only problematic if IBar is existential-typed)
// This case is deferred per issue #9835 discussion.
struct OkImpl : IFoo
{
    IBar other;  // OK - different interface, no self-referential recursion
    void foo() {}
};

[numthreads(1, 1, 1)]
void computeMain()
{
}
