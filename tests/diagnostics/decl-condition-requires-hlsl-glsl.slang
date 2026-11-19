//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/decl-condition-requires-hlsl-glsl.slang -no-codegen -warnings-disable 30081

// The compiler rejects a declaration used as a condition when parsing the
// GLSL dialect, with the same dedicated diagnostic as for the Slang language.
// The declaration is still parsed for error recovery.

void main()
{
    int c = 1;
    if (int count = c)
    /*CHECK:
        ^^^ a variable declaration is not allowed as a condition
    */
    {}
}
