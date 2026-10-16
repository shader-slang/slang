// The unary `+` operator is an identity operator and must be folded like other unary operators.
// Previously, it was implemented as a hand-written `operator+` without an intrinsic operator.
// Now that constant folding can handle this operator, an expression remains constant even if it
// contains a single `+` within a const-expr.
//TEST:INTERPRET(filecheck=CHECK):

static const int kCount = 3;
static const int kNegated = -+kCount;
static const uint kUnsigned = +7u;

void main()
{
    int a[+kCount];
    uint b[kUnsigned + 2u];
    //CHECK: 3 -3 9
    printf("%d %d %d\n", a.getCount(), kNegated, b.getCount());

    // Unary `+` on vectors and matrices is generated from the same table entry as the scalar
    // form, so it must likewise leave its operand untouched.
    float3 v = +float3(1, 2, 3);
    float2x2 m = +float2x2(4, 5, 6, 7);
    int3 iv = +int3(8, 9, 10);
    //CHECK: 1 2 3 4 7 8 10
    printf(
        "%d %d %d %d %d %d %d\n",
        (int)v.x,
        (int)v.y,
        (int)v.z,
        (int)m[0][0],
        (int)m[1][1],
        iv.x,
        iv.z);
}
