//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Slang loop variables stay scoped to their own loops. Sibling loop declarations and an
// ordinary declaration after a loop are valid, but a post-loop read needs an outer declaration.

void siblingLoopsAndLaterDeclaration()
{
    for (int i = 0; i < 2; ++i)
    {}
    for (int i = 0; i < 3; ++i)
    {}
    int i = 4;
}

int outerVariable()
{
    int i = 7;
    for (int i = 0; i < 2; ++i)
    {}
    return i;
}

int readAfterLoop()
{
    for (int i = 0; i < 2; ++i)
    {}
    return i;
/*CHECK:
           ^ undefined identifier
           ^ undefined identifier 'i'.
*/
}
