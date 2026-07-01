// Regression test for:
// https://github.com/shader-slang/slang/issues/11316
//
// Using a value typed by a global-scope generic parameter in a shader body
// used to crash deep inside specialization with an assertion / SIGSEGV. The
// construct is unsupported by policy (#5627); the fix is a clean E38207
// diagnostic.
//
// Three directives cover the distinct lowerings:
//  - `main`: a *constrained* `type_param T : IFoo` whose value is used via an
//    interface-method call (the original #11316 reproducer). The constraint
//    lowers to a type-kind param plus a paired witness-table param; dedup by
//    source location yields a single E38207.
//  - `mainUnconstrained`: an *unconstrained* `type_param U` used directly,
//    exercising the same path with no witness-table param.
//  - `mainValueParam`: a `__generic_value_param`, which lowers to a *non*
//    type-kind `IRGlobalGenericParam`; it must also be diagnosed (otherwise the
//    leftover-use release assert in specialization would fire).
//  - `mainConjunction`: a *multi-constraint* `type_param TT : IFoo & IBar`,
//    which lowers to a type-kind param plus *two* witness-table params. A plain
//    FileCheck directive pins that exactly one E38207 is reported (not one per
//    witness param), guarding the dedup.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKU,non-exhaustive):-target spirv-asm -entry mainUnconstrained -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKV,non-exhaustive):-target spirv-asm -entry mainValueParam -stage compute
//TEST:SIMPLE(filecheck=CHECKC):-target spirv-asm -entry mainConjunction -stage compute

interface IFoo
{
    int get();
}

interface IBar
{
    int bar();
}

RWStructuredBuffer<int> outputBuffer;

type_param T : IFoo;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    T t;
    int v = t.get();
//CHECK: E38207
//CHECK: cannot be used in shader code without a concrete binding
}

type_param U;

[shader("compute")]
[numthreads(1, 1, 1)]
void mainUnconstrained()
{
    U u;
    outputBuffer[0] = reinterpret<int>(u);
//CHECKU: E38207
//CHECKU: cannot be used in shader code without a concrete binding
}

__generic_value_param N : int;

[shader("compute")]
[numthreads(1, 1, 1)]
void mainValueParam()
{
    outputBuffer[0] = N;
//CHECKV: E38207
//CHECKV: cannot be used in shader code without a concrete binding
}

type_param TT : IFoo & IBar;

[shader("compute")]
[numthreads(1, 1, 1)]
void mainConjunction()
{
    TT t;
    outputBuffer[0] = t.get() + t.bar();
}
// The conjunction lowers to one type-kind param plus two witness-table params,
// but the dedup must collapse them to exactly one E38207.
//CHECKC-COUNT-1: error{{.*}}38207
//CHECKC-NOT: error{{.*}}38207
