//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A local declared as the unbraced body of a statement is not visible before
// its declaration, so a forward reference with no outer declaration of the
// same name is undefined (gh-14118).

void ifBody(bool c)
{
    int v = a;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'a'.
    if (c)
        int a = 1;
}

void elseBody(bool c)
{
    int v = b;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'b'.
    if (c)
    {
    }
    else
        int b = 1;
}

void whileBody(bool c)
{
    int v = d;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'd'.
    while (c)
        int d = 1;
}

void doBody(bool c)
{
    int v = e;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'e'.
    do
        int e = 1;
    while (c);
}

void labeledDecl()
{
    int v = f;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'f'.
inner:
    int f = 1;
}

void forBody()
{
    for (int k = 0; k < g; k++)
//CHECK:                ^ undefined identifier
//CHECK:                ^ undefined identifier 'g'.
        int g = 1;
    for (int k = 0; k < 1; k += h)
//CHECK:                        ^ undefined identifier
//CHECK:                        ^ undefined identifier 'h'.
        int h = 1;
}
