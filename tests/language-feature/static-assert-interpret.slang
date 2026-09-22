// A passing `static_assert` (module scope and function-body scope) must not disturb execution on
// the interpreter (HostVM) target: it leaves no `kIROp_StaticAssert` or carrier for the VM emitter.

//TEST:INTERPRET(filecheck=CHECK):
static_assert(1 + 1 == 2, "module scope holds");
static_assert(3 > 2);

void main()
{
    static_assert(4 == 2 + 2);
    //CHECK: interpreter-ran
    printf("interpreter-ran\n");
}
