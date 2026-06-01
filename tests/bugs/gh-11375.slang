// Regression for shader-slang/slang#11375 — slangi VM was aborting with
// "VM operand access out of bounds in constants section" when a `bool`
// literal flowed through `ByteCodeEmitter::addConstantValue`, because the
// switch in source/slang/slang-emit-vm.cpp had no `kIROp_BoolLit` case
// and no `default:` arm — the operand reserved space in the constants
// section without any backing bytes, so the runtime bounds check
// (added in PRs #11261 / #11309) tripped on the next read.

//TEST:INTERPRET(filecheck=CHECK):

bool bump(inout int counter)
{
    counter++;
    return true;
}

void main()
{
    int counter = 0;
    bool r1 = (false) && bump(counter);
    // CHECK: counter-after-false-and=0
    printf("counter-after-false-and=%d\n", counter);

    bool r2 = (true) && bump(counter);
    // CHECK: counter-after-true-and=1
    printf("counter-after-true-and=%d\n", counter);

    bool r3 = (true) || bump(counter);
    // CHECK: counter-after-true-or=1
    printf("counter-after-true-or=%d\n", counter);

    bool r4 = (false) || bump(counter);
    // CHECK: counter-after-false-or=2
    printf("counter-after-false-or=%d\n", counter);

    // Plain bool literals must also round-trip through the constants section.
    bool t = true;
    bool f = false;
    // CHECK: t=1 f=0
    printf("t=%d f=%d\n", (int)t, (int)f);
}
