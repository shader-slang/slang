//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

RWStructuredBuffer<float> buf;

//
// This test checks that we correctly diagnose trying to use the assignment and
// typed assignment syntax with operands with no result type or id.
//
int foo(const int constParam)
{
    spirv_asm
    {
        %bar : %wrong = OpTypeInt 32 0
        // CHECK: wrong-assignment-opcode.slang([[#@LINE-1]]): error 29104: cannot use this 'x : <type> = OpTypeInt...' syntax because OpTypeInt does not have a <result-type-id> operand

    };
    spirv_asm
    {
        %foo = OpStore 1 2;
        // CHECK: wrong-assignment-opcode.slang([[#@LINE-1]]): error 29104: cannot use this 'x = OpStore...' syntax because OpStore does not have a <result-id> operand
    };
}
