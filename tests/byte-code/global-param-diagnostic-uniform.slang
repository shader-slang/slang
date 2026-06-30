// A program that references a global shader parameter (here a plain uniform)
// cannot be run by the slangi CPU interpreter. The interpreter should report a
// clean diagnostic (error 52013) explaining the limitation, rather than aborting
// with an internal error (error 99999/99997). See issue #10677.
//
// This exercises the ensureInst detection path: a uniform read lowers to a
// global-scope load of the parameter, so the unsupported value reaching value
// emission is the load rather than the parameter itself, and findReferencedGlobalParam
// recovers the underlying parameter. Complements global-param-diagnostic.slang,
// which exercises the emitFunction pre-scan path.

//TEST:INTERPRET(filecheck=CHECK):

uniform float gScale;

void main()
{
    // A non-zero result code confirms the diagnostic propagated as a clean
    // compilation failure (SLANG_FAIL) rather than producing usable bytecode.
    //CHECK: result code = 1
    //CHECK: error[E52013]
    //CHECK-SAME: not supported by the Slang interpreter
    //CHECK-SAME: compile this program with slangc
    printf("%f\n", gScale * 2.0f);
}
