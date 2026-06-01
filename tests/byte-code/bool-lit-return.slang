//TEST:INTERPRET(filecheck=CHECK):

// Regression test for #11375: slangi VM used to crash on any bool-returning
// function with a literal `return true;` / `return false;` body, because
// the VM emitter's addConstantValue switch had no case for kIROp_BoolLit
// and the constant section was left short of the operand size.

bool returnTrue() { return true; }
bool returnFalse() { return false; }

void main()
{
    //CHECK: T1
    if (returnTrue()) { printf("T1\n"); } else { printf("F1\n"); }
    //CHECK: F2
    if (returnFalse()) { printf("T2\n"); } else { printf("F2\n"); }
}
