//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv

// Regression guard: calling a mutating method on an `in` parameter of type Atomic<T>
// (or a struct containing one) must be a hard error (E30067), not just a warning.
//
// Before gh-7262, Atomic<T> was copyable, so this fell through to the E30068 warning
// branch in slang-check-overload.cpp. After marking Atomic<T> as [__NonCopyableType],
// isNonCopyableType() returns true for the parameter type, and the error branch fires.

void f(Atomic<int> a)
{
    a.add(1);
//CHECK: error[E30067]: mutating method called on `in` parameter
//CHECK: 'add' called on `in` parameter 'a'
}
