// Regression test for #10751: a user-defined unary operator overload
// (`operator-`, `operator+`, `operator~`, `operator!`) should be usable in
// prefix position without an explicit `__prefix` modifier. Semantic checking
// infers the `__prefix` fixity from the operator name and its unary arity.
//
// Also checks that a binary `operator-(A, A)` declared alongside the unary
// `operator-(A)` is not misinterpreted as a prefix operator.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

struct A
{
    int x;
}

A operator-(A a)
{
    A r = {-a.x};
    return r;
}

A operator~(A a)
{
    A r = {~a.x};
    return r;
}

A operator!(A a)
{
    A r = {a.x == 0 ? 1 : 0};
    return r;
}

A operator+(A a)
{
    A r = {a.x};
    return r;
}

// Binary subtraction must keep working next to the unary `operator-(A)`.
A operator-(A a, A b)
{
    A r = {a.x - b.x};
    return r;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a = {5};
    A b = {2};
    outputBuffer[0] = (-a).x;    // -5
    outputBuffer[1] = (~a).x;    // ~5 == -6
    outputBuffer[2] = (!a).x;    // 0
    outputBuffer[3] = (+a).x;    // 5
    outputBuffer[4] = (a - b).x; // 3
    // CHECK: -5
    // CHECK-NEXT: -6
    // CHECK-NEXT: 0
    // CHECK-NEXT: 5
    // CHECK-NEXT: 3
}
