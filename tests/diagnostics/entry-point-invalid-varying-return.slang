// Test that invalid types are rejected as entry-point return types.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry atomicReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=DIFFPAIR):-target spirv -entry diffPairReturn -stage vertex

// Atomic<T> as a return type
[shader("vertex")]
Atomic<int> atomicReturn()
//CHECK: ^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying parameter or return type
{
    return {};
}

// DifferentialPair<T> as a return type
[shader("vertex")]
DifferentialPair<float> diffPairReturn()
//DIFFPAIR: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying parameter or return type
{
    return {};
}
