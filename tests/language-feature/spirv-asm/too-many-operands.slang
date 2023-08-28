//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

RWStructuredBuffer<float> buf;

//
// This test checks that we correctly diagnose passing too many operands to a spriv inst
//
void foo(const int constParam)
{
    spirv_asm
    {
        // Make sure we diagnose the first offending operand
        OpLoad
            %a
            %b
            %c
            %d
            %e
        // CHECK: too-many-operands.slang([[#@LINE-1]]): warning 29104: too many operands for OpLoad (expected max 4), did you forget a semicolon?
            %f;

        %r : $$int = OpIAdd %r %r // oops, I forgot the semicolon
        OpNop
        // CHECK: too-many-operands.slang([[#@LINE-1]]): warning 29104: too many operands for OpIAdd (expected max 4), did you forget a semicolon?
    };
}
