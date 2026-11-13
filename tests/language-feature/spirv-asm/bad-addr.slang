//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

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
//CHECK:                   ^^^ error E29103
        OpLoad $$int result &constParam;
//CHECK:                     ^^^^^^^^^^ error E29103
    };
}
