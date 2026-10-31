//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_NOABORT):-target spirv -entry computeMain -stage compute

// The `result` marker names the result of the instruction it appears in, so it can only be
// that instruction's <result-id> operand. In an instruction with no result id, such as an
// execution mode, or as an ordinary operand, it is an error rather than a crash of the
// emitter.

// CHECK_NOABORT: error{{.*}}E29119
// CHECK_NOABORT-NOT: InternalError
// CHECK_NOABORT-NOT: assert

RWStructuredBuffer<int> outputBuffer;

void executionMode()
{
    spirv_asm
    {
        OpExecutionMode __entryPoint LocalSize result 1 1
//CHECK:                                       ^^^^^^ error E29119
//CHECK:                                       ^^^^^^ 'result' can only be the <result-id> operand of an instruction, and this operand of OpExecutionMode is not its <result-id>
    };
}

void executionModeId()
{
    spirv_asm
    {
        %one = OpConstant $$uint 1;
        OpExecutionModeId $computeMain LocalSizeId result %one %one
//CHECK:                                           ^^^^^^ error E29119
//CHECK:                                           ^^^^^^ 'result' can only be the <result-id> operand of an instruction, and this operand of OpExecutionModeId is not its <result-id>
    };
}

void constantOperand()
{
    spirv_asm
    {
        %one = OpConstant $$int 1;
        %v = OpConstantComposite $$int2 %one result
//CHECK:                                     ^^^^^^ error E29119
//CHECK:                                     ^^^^^^ 'result' can only be the <result-id> operand of an instruction, and this operand of OpConstantComposite is not its <result-id>
    };
}

[numthreads(1, 1, 1)]
void computeMain()
{
    executionMode();
    executionModeId();
    constantOperand();
    outputBuffer[0] = 1;
}
