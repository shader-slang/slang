//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/decl-condition-requires-hlsl-glsl.slang -no-codegen -warnings-disable 30081

// The GLSL dialect rejects a declaration used as a condition with the same dedicated
// diagnostic as the Slang dialect (the acceptance is HLSL-flavored-dialect only).
// The declaration is still parsed for error recovery. The 30081 warning is disabled:
// the `int` condition converts to `bool` by design, and the recovery parse reports
// that conversion like any other.

void main()
{
    int c = 1;
    if (int count = c)
    /*CHECK:
        ^^^ a declaration cannot be used as a condition
        ^^^ a variable declaration can only be used as the condition of an `if`, `while`, or `do`-`while` statement in the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
    */
    {}
}
