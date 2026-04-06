//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `case` keyword outside an enum body is not parsed as an EnumCaseDecl.
// The parser treats it as an undefined identifier.

struct S
{
    case BadCase = 1;
/*CHECK:
    ^^^^ undefined identifier
    ^^^^ undefined identifier 'case'.
*/
}
