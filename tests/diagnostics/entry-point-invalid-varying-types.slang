// Test that invalid types are rejected as entry-point varying parameters.
//
// Covers the parameter side of:
//   #9429 DifferentialPair<T>
//   #9443 Atomic<T>
//   #9446 CoopVec<T, 0>  (zero-sized)
//   #9448 CoopVec<T, N>  (general)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry diffPairParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=DIFFPTR,non-exhaustive):-target spirv -entry diffPtrPairParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ATOMIC,non-exhaustive):-target spirv -entry atomicParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=COOPVEC,non-exhaustive):-target spirv -entry coopVecParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=COOPVEC0,non-exhaustive):-target spirv -entry coopVecZeroParam -stage vertex

// DifferentialPair<T> as a varying parameter
[shader("vertex")]
float4 diffPairParam(DifferentialPair<float> dp : A) : SV_Position
//CHECK: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}

// A user-defined type that conforms to IDifferentiablePtrType, so we can
// exercise the sibling DifferentialPtrPairType branch of the match predicate.
struct DPtrFloat : IDifferentiablePtrType
{
    typealias Differential = DPtrFloat;
    float* ptr;
}

// DifferentialPtrPair<T> as a varying parameter (backward-mode autodiff type)
[shader("vertex")]
float4 diffPtrPairParam(DifferentialPtrPair<DPtrFloat> dp : A) : SV_Position
//DIFFPTR: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}

// Atomic<T> as a varying parameter
[shader("vertex")]
float4 atomicParam(Atomic<int> a : A) : SV_Position
//ATOMIC: Atomic is not a valid varying type
{
    return float4(0);
}

// CoopVec<T, N> as a varying parameter (general case, issue #9448)
[shader("vertex")]
float4 coopVecParam(CoopVec<float, 4> v : A) : SV_Position
//COOPVEC: CoopVec is not a valid varying type for this stage
{
    return float4(0);
}

// CoopVec<T, 0> as a varying parameter (zero-sized case, issue #9446)
[shader("vertex")]
float4 coopVecZeroParam(CoopVec<int, 0> v : A) : SV_Position
//COOPVEC0: CoopVec is not a valid varying type for this stage
{
    return float4(0);
}
