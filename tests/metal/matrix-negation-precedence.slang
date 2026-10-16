//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain

// Metal (MSL) has no unary '-' on matrices, so the Metal emitter lowers matrix negation to the
// subtraction "(matrix(0) - operand)". "operand" is the right-hand side of that subtraction, so
// an additive operand such as "m1 + m2" -- equal precedence to '-', and the RHS of the
// left-associative '-' -- must be wrapped in its own parentheses: "-(m1 + m2)" must emit
// "(matrix(0) - (m1 + m2))". Without the inner parentheses the expression reassociates to
// "-m1 + m2" (i.e. m2 - m1), silently changing the result. Vectors and scalars are unaffected:
// the branch guards matrices only, so they take the shared base-emitter negation path (a real
// unary '-').
//
// Inputs are derived from SV_DispatchThreadID so the negation is applied to runtime values;
// negation of a compile-time constant is folded away before emission and would not exercise this
// path.

RWStructuredBuffer<float2x2> outMat;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float2x2 m1 = float2x2(float(tid.x), 1, 2, 3);
    float2x2 m2 = float2x2(float(tid.y), 4, 5, 6);

    //METAL: (0) - (m1{{.*}} + m2
    outMat[0] = -(m1 + m2);

    //METAL: (0) - (m1{{.*}} - m2
    outMat[1] = -(m1 - m2);
}
