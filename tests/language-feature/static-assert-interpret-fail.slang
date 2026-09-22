// A failing `static_assert` must be reported, and must stop execution, on the interpreter (HostVM)
// target — both at module scope and inside a function body.

//TEST:INTERPRET(filecheck=CHECK):
static_assert(1 == 2, "MODULE_one_is_not_two");
//CHECK-DAG: static assertion failed, MODULE_one_is_not_two

void main()
{
    static_assert(2 + 2 == 5, "FUNC_two_plus_two_is_not_five");
    //CHECK-DAG: static assertion failed, FUNC_two_plus_two_is_not_five
    //CHECK-NOT: should-not-run
    printf("should-not-run\n");
}
