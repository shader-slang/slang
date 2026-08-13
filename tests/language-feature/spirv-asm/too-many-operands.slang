//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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
//CHECK:     ^ undefined SPIR-V id
//CHECK:     ^ SPIRV id '%a' is not defined in the current assembly block location
            %b
            %c
//CHECK:     ^ undefined SPIR-V id
//CHECK:     ^ SPIRV id '%c' is not defined in the current assembly block location
            %d
//CHECK:     ^ undefined SPIR-V id
//CHECK:     ^ SPIRV id '%d' is not defined in the current assembly block location
        // The `warning` severity checked below is deliberate: E29106 is the
        // parser's recovery guess (missing semicolon), so it stays a warning.
        // The zero-operand case is a hard error (E29118) instead — see
        // too-many-operands-zero-operand-op.slang and shader-slang/slang#12497.
        // Unifying the two would silently reintroduce the abort.
        %r : $$int = OpIAdd %r %r
//CHECK:^ warning E29106
//CHECK:^ too many operands for OpLoad (expected max 4), did you forget a semicolon?
//CHECK:   ^ unexpected token
//CHECK:   ^ unexpected ':'
//CHECK: ^ undefined SPIR-V id
//CHECK: ^ SPIRV id '%r' is not defined in the current assembly block location
    };
}
