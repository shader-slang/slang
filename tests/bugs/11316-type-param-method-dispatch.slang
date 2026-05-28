// Regression test for:
// https://github.com/shader-slang/slang/issues/11316
//
// Calling an interface method on a value typed by a constrained
// module-scope `type_param` (`type_param T : IFoo;`, then `T t; t.get();`
// in a shader body) used to crash deep inside specialization with an
// assertion / SIGSEGV. The construct is unsupported by policy (#5627);
// the fix is a clean E38207 diagnostic that names it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

interface IFoo
{
    int get();
}

type_param T : IFoo;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    T t;
    int v = t.get();
}

//CHECK: E38207
//CHECK: type_param
