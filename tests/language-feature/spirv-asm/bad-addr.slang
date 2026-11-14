//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

RWStructuredBuffer<float> buf;

//
// This test checks that we correctly diagnose trying to use the address of
// something without one
//
int foo(const int constParam)
{
    return spirv_asm
    {
        OpLoad $$int %foo &buf;
//CHECK:                   ^^^ unable to take the address of this address-of asm operand
        OpLoad $$int result &constParam;
//CHECK:                     ^^^^^^^^^^ unable to take the address of this address-of asm operand
    };
}
