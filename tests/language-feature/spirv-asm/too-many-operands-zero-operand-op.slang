//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//TEST:SIMPLE(filecheck=CHECK_NOABORT):-target spirv

RWStructuredBuffer<float> buf;

// Giving an operand to a zero-operand SPIR-V opcode (OpNop, OpReturn, ...) must
// report an error and stop cleanly, not abort the compiler.

// CHECK_NOABORT runs over the full compiler output, so the CHECK-NOT rules —
// which follow the positive match, where an abort message would appear — fail
// if the error is emitted but lowering still aborts on the error type.
//CHECK_NOABORT: error{{.*}}E29118
//CHECK_NOABORT-NOT: InternalError
//CHECK_NOABORT-NOT: unexpected: ErrorType

void foo()
{
    spirv_asm
    {
        OpNop %buf;
//CHECK:^^^^^ error E29118
//CHECK:^^^^^ OpNop does not take any operands
    };

    spirv_asm
    {
        OpReturn %buf;
//CHECK:^^^^^^^^ error E29118
//CHECK:^^^^^^^^ OpReturn does not take any operands
    };
}
