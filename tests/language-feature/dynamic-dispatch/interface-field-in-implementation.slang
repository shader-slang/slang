//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Test for issue #9835: An implementation of IFoo should NOT be allowed
// to contain IFoo or IFoo[] typed fields (directly or indirectly), as this
// creates infinite recursion in the existential representation.

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

// Case 3: Indirect self-reference through nested struct - should error
struct InnerWithIFoo
{
    IFoo nested;
};

struct BadImpl3 : IFoo
{
    // CHECK: ([[# @LINE+1]]): error 33083: cannot have interface-typed member 'wrapper' when conforming to interface 'IFoo'.
    InnerWithIFoo wrapper;  // Contains IFoo indirectly
    void foo() {}
};

// Case 4: Different interface - allowed (no self-referential recursion)
struct OkImpl : IFoo
{
    IBar other;  // OK - different interface
    void foo() {}
};

// Case 5: Struct without target interface - allowed
struct InnerWithIBar
{
    IBar nested;
};

struct OkImpl2 : IFoo
{
    InnerWithIBar wrapper;  // OK - doesn't contain IFoo
    void foo() {}
};

[numthreads(1, 1, 1)]
void computeMain()
{
}
