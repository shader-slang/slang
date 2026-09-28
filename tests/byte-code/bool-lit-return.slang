//TEST:INTERPRET(filecheck=CHECK):

// Regression test for #11375: a `bool` literal misbehaved under slangi (bytecode VM)
// because ByteCodeEmitter::addConstantValue had no kIROp_BoolLit arm, so it left the
// reserved constant slot unbacked. A trailing bool constant (a `return true;` /
// `return false;` body, as here) then read past the constant section on Ret and
// crashed; a non-trailing bool constant instead produced a silent wrong value. This
// test covers the trailing return-path crash; the argument-path silent-wrong-value
// case is covered in tests/bugs/gh-11402.slang.

bool returnTrue() { return true; }
bool returnFalse() { return false; }

void main()
{
    //CHECK: T1
    if (returnTrue()) { printf("T1\n"); } else { printf("F1\n"); }
    //CHECK: F2
    if (returnFalse()) { printf("T2\n"); } else { printf("F2\n"); }
}
