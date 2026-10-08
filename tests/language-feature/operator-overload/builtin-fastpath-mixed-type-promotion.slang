// The mixed-type fast path (`unifyBaseType` / `coerceOperandsOfBuiltinBinaryExpr`) must select
// the SAME common operand type that generic `operator OP` overload resolution would. Slang's
// usual-arithmetic-conversions pick the float operand even when it is *smaller* than the int
// operand, because `int -> half` costs less than `int -> float` plus `half -> float`; so
// `int + half` resolves to `half`, not `float`. (Verified: disabling the fast path produces the
// identical selections.) This pins that the hard-coded promotion table does not diverge.
//TEST:INTERPRET(filecheck=CHECK):

int which(half v)
{
    return 1;
}
int which(float v)
{
    return 2;
}
int which(double v)
{
    return 3;
}

void main()
{
    int i = 5;
    half h = 2.0h;
    int64_t i64 = 5;
    uint u = 3u;
    printf("%d\n", which(i + h)); // int   + half -> half
    //CHECK: 1
    printf("%d\n", which(i64 + h)); // int64 + half -> half
    //CHECK: 1
    printf("%d\n", which(u + h)); // uint  + half -> half
    //CHECK: 1
    printf("%d\n", which(i + 1.0f)); // int   + float -> float
    //CHECK: 2
}
