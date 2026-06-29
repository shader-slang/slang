// Programs that reference a global shader parameter or a GPU resource type
// cannot be run by the slangi CPU interpreter. The interpreter should report a
// clean diagnostic (error 52013) explaining the limitation, rather than aborting
// with an internal error (error 99999/99997). See issue #10677.

//TEST:INTERPRET(filecheck=CHECK):

RWStructuredBuffer<float> inputs;

float load(uint index)
{
    return inputs[index];
}

void main()
{
    //CHECK: error[E52013]
    //CHECK-SAME: not supported by the Slang interpreter
    //CHECK-SAME: compile this program with slangc
    printf("%f\n", load(0));
}
