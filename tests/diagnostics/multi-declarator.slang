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

    // A type specifier with at least one declarator forms a group, and the
    // group's shared modifiers are not diagnosed on the type. This pins the
    // `SharedModifiers` marker that `DeclGroupBuilder::beginGroup` adds.
    static struct L { int x; };
/*CHECK:
    ^^^^^^ modifier not allowed
    ^^^^^^ modifier 'static' is not allowed here.
*/
    static struct M { int x; } m0;
    static struct N { int x; } n0, n1;

    buf[0] = j + b + m0.x + n0.x + n1.x;
}

// The declarators of a group are not visible before their statement.
int useBeforeGroup()
{
    int y = j;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'j'.
    int j = 1, k = 2;
    return y + j + k;
}

int useBeforeGroupWithLessThan()
{
    int y = j;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'j'.
    int j = 1, k = j < 2;
    return y + j + k;
}
