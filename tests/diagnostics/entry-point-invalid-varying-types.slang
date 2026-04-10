// Test that invalid types are rejected as entry-point varying parameters.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry diffPairParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ATOMIC):-target spirv -entry atomicParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=COOPVEC):-target spirv -entry coopVecParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=MATRIX):-target spirv -entry matrixParam -stage vertex

// DifferentialPair<T> as a varying parameter
[shader("vertex")]
float4 diffPairParam(DifferentialPair<float> dp : A) : SV_Position
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying parameter or return type
{
    return float4(0);
}

// Atomic<T> as a varying parameter
[shader("vertex")]
float4 atomicParam(Atomic<int> a : A) : SV_Position
//ATOMIC: ^^^^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying parameter or return type
{
    return float4(0);
}

// CoopVec<T, N> as a varying parameter
[shader("vertex")]
float4 coopVecParam(CoopVec<float, 4> v : A) : SV_Position
//COOPVEC: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying parameter or return type
{
    return float4(0);
}

// matrix with non-standard element type as a SPIR-V varying parameter
[shader("vertex")]
float4 matrixParam(matrix<uint64_t, 2, 2> m : A) : SV_Position
//MATRIX: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be used as entry-point varying for this target
{
    return float4(0);
}
