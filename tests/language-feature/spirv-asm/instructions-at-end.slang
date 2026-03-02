//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-emit-spirv-directly -target spirv-asm -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//
// This test checks that we correctly diagnose using 'result' in an instruction
// not in the last position.
//
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    int n = outputBuffer[i];
    int r = spirv_asm
//CHECK:    ^^^^^^^^^ type mismatch in expression
//CHECK:    ^^^^^^^^^ expected an expression of type 'int', got 'void'
    {
        OpConstant $$int %two 2;
        OpIMul $$int result $n %two;
//CHECK:             ^^^^^^ misplaced result-id marker
//CHECK:             ^^^^^^ the result-id marker must only be used in the last instruction of a spriv_asm expression

        OpIAdd $$uint %something %two %two;
    };
    outputBuffer[i] = r;
}
//CHECK: consider adding an OpCopyObject instruction to the end of the spirv_asm expression
