// non-exhaustive: `Test t;` also produces "use of uninitialized variable" warnings;
// those are irrelevant to this regression guard and are allowed but not required.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

// Regression guard: performing an atomic operation on a local variable of a struct
// containing Atomic<T> is an error (E41403). The local variable lands in Function
// address space, which is an invalid atomic destination — neither groupshared nor
// a device buffer.
//
// Before gh-7262, this was silently accepted and produced invalid SPIR-V (an atomic
// op targeting a Function-address-space pointer).

struct Test {
    Atomic<int> test;
}

[shader("compute")]
[numthreads(1, 1, 1)]
public void main() {
    Test t;
    t.test.increment();
//CHECK: invalid atomic destination
//CHECK: cannot perform atomic operation because destination is neither groupshared nor from a device buffer
}
