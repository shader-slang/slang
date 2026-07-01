//TEST:INTERPRET(filecheck=CHECK):

// Regression for shader-slang/slang#11402 — under slangi (bytecode VM) a `bool`
// literal passed as a *function argument* was read back as `true` inside the
// callee. Same root cause as #11375 (missing kIROp_BoolLit arm in
// ByteCodeEmitter::addConstantValue left the reserved constant slot unbacked),
// but here the bool constant is non-trailing, so instead of an out-of-bounds
// crash it produced a silent wrong value: fn(false) took the true branch.
// Complements the return-path coverage in tests/byte-code/bool-lit-return.slang
// by exercising the argument-passing path.

int fn(bool x)
{
    if (x)
        return 1;
    else
        return 2;
}

void main()
{
    // CHECK: a=1 b=2
    printf("a=%d b=%d\n", fn(true), fn(false));
}
