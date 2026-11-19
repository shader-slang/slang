//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): tests/diagnostics/decl-condition-requires-hlsl.slang -no-codegen -warnings-disable 30081

// The compiler rejects a declaration used as a condition with one dedicated
// diagnostic, not a cascade of unrelated parse errors. The declaration is
// still parsed for error recovery, so recovery continues with one clear
// message.

void main()
{
    int c = 1;
    if (int count = c)
    /*CHECK:
        ^^^ a variable declaration is not allowed as a condition
    */
    {}
    while (int count = c)
    /*CHECK:
           ^^^ a variable declaration is not allowed as a condition
    */
    {}
    do
    {
    } while (int count = c);
    /*CHECK:
             ^^^ a variable declaration is not allowed as a condition
    */
}
