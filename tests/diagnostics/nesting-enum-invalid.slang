//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Non-static member variable inside an enum body is caught by the parser
// as an unexpected token (enum body expects case declarations).

enum E
{
    A,
    B,

    int bad_field;
//CHECK:^^^^^^^^^ unexpected token
//CHECK:^^^^^^^^^ unexpected identifier, expected ','
}
