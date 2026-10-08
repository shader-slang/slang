//TEST:INTERPRET(filecheck=CHECK):

// A program that references a GPU resource global (here a RWStructuredBuffer)
// cannot be run by the slangi CPU interpreter. The interpreter should report a
// clean diagnostic (error 52013) explaining the limitation, rather than aborting
// with an internal error (error 99999/99997). See issue #10677.
//
// This exercises the pre-scan detection path in emitFunction: the structured-buffer
// access lowers to an instruction whose handler aborts before reaching ensureInst,
// so the global parameter is caught as a direct operand up front. The buffer is
// referenced twice to confirm the diagnostic is reported once per parameter.

RWStructuredBuffer<float> inputs;

float load(uint index)
{
    return inputs[index];
}

void main()
{
    // A non-zero result code confirms the diagnostic propagated as a clean
    // compilation failure (SLANG_FAIL) rather than producing usable bytecode.
    //CHECK: result code = 1
    // The old internal-error path (E99999/E99997) must stay gone.
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    // The legalized parameter name is substituted into the message (the '~name'
    // placeholder renders a real, non-empty name).
    //CHECK: error[E52013]
    //CHECK-SAME: 'globalParams'
    //CHECK-SAME: not supported by the Slang interpreter
    //CHECK-SAME: compile this program with slangc
    //CHECK-NOT: error[E52013]
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    printf("%f\n", load(0) + load(1));
}
