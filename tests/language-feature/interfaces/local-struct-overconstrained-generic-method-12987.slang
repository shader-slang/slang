// A function-local struct whose generic method is more constrained than the interface
// requirement (`f<T : IA & IB>` vs `f<T : IA>`) must be rejected with a requirement-mismatch
// diagnostic, exactly as the equivalent module-scope struct is -- neither crashing nor being
// silently accepted. The conjunction constraint is flattened to two constraint decls before
// signature matching, so the extra `IB` bound makes the constraint count differ from the
// single-constraint requirement, and the signatures mismatch (E38105). Regression test for
// shader-slang/slang#12987.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry computeMain

interface IA {}
interface IB {}
interface IReq { void f<T : IA>(); }

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    struct Op : IReq { void f<T : IA & IB>() {} };
    output[0] = 0;
}

//CHECK: E38105
