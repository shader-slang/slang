//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain

// Diagnostics for multi-declarator local declarations (gh-13428).

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void computeMain()
{
    // A declarator's name is in scope only after its own declarator, so an
    // initializer that refers to the variable being declared is still an
    // error, even though earlier declarators of the same statement are visible.
    int j = j < 2;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'j'.
    int a = buf[0], b = b < a;
//CHECK:                ^ undefined identifier
//CHECK:                ^ undefined identifier 'b'.

    // The declarators of a group are not visible before their statement.
    int y0 = c0;
//CHECK:     ^^ undefined identifier
//CHECK:     ^^ undefined identifier 'c0'.
    int c0 = 1, c1 = 2;
    int y1 = d0;
//CHECK:     ^^ undefined identifier
//CHECK:     ^^ undefined identifier 'd0'.
    int d0 = 1, d1 = d0 < 2;

    // Modifiers written before a type specifier that declares a type apply to
    // the declarators, so they are not diagnosed on the type when at least one
    // declarator follows.
    static struct L { int x; };
/*CHECK:
    ^^^^^^ modifier not allowed
    ^^^^^^ modifier 'static' is not allowed here.
*/
    static struct M { int x; } m0;
    static struct N { int x; } n0, n1;

    buf[0] = j + b + m0.x + n0.x + n1.x + y0 + c1 + y1 + d1;
}
