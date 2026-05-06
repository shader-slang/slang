//TEST:INTERPRET(filecheck=CHECK):

// Companion to tests/diagnostics/inout-constant-arg.slang.
// Exercises the *positive* path of the implicit-cast-as-lvalue
// mechanism that #9700's fix tightens: when the argument really is an
// l-value of a coercible-but-different integer type, the call should
// still compile and the back-and-forth conversion should round-trip.

void inc(inout uint x) { x = x + 1; }
void writeFive(out uint x) { x = 5; }

void main()
{
    int a = 1;
    inc(a); // inout: implicit-cast-as-lvalue: int <-> uint, a is an lvalue
    //CHECK: a=2
    printf("a=%d\n", a);

    int b = 0;
    writeFive(b); // out: same coercion path on the write-back direction
    //CHECK: b=5
    printf("b=%d\n", b);
}
