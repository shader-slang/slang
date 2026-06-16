// Negative regression tests for the builtin-operator fast path
// (`convertToBuiltinArithmeticOp`):
//   1. A user-defined `operator+` on a user struct must still be selected (the fast path is
//      gated on builtin scalar/vector/matrix operands and must not hijack user overloads).
//   2. `&&` must keep short-circuit semantics: the fast path bails on it (it is neither
//      arithmetic, comparison, nor bitwise) and `convertToLogicOperatorExpr` runs first, so the
//      right-hand side is not evaluated when the left is false.
//TEST:INTERPRET(filecheck=CHECK):

struct V
{
    int x;
}

// Sentinel multiplier (`* 1000`) makes a user-overload result distinguishable from any builtin add.
V operator+(V a, int b)
{
    V r;
    r.x = a.x + b * 1000;
    return r;
}

bool sideEffect()
{
    printf("called\n");
    return true;
}

void main()
{
    V v;
    v.x = 3;
    V r = v + 5; // must select the user operator+: 3 + 5*1000 = 5003
    printf("user=%d\n", r.x);
    //CHECK: user=5003

    bool b = false && sideEffect(); // must short-circuit: sideEffect() not evaluated
    //CHECK-NOT: called
    printf("sc=%d\n", b ? 1 : 0);
    //CHECK: sc=0
}
