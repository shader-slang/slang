//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK):-lang hlsl tests/diagnostics/hlsl-for-loop-declaration-order.slang

// Only a new loop initializer may redeclare an existing variable in the same HLSL scope.
// Ordinary duplicate declarations remain errors, and loop variables are hidden until declared.

void ordinaryRedeclaration()
{
    int value = 1;
/*CHECK:
        ^^^^^ see previous declaration of 'value'
*/
    int value = 2;
/*CHECK:
        ^^^^^ conflicting declaration
        ^^^^^ declaration of 'value' conflicts with existing declaration
*/
}

void ordinaryDeclarationAfterLoop()
{
    for (int i = 0; i < 2; ++i)
/*CHECK:
             ^ see previous declaration of 'i'
*/
    {}
    int i = 5;
/*CHECK:
        ^ conflicting declaration
        ^ declaration of 'i' conflicts with existing declaration
*/
}

void readBeforeLoop()
{
    int value = i;
/*CHECK:
                ^ undefined identifier
                ^ undefined identifier 'i'.
*/
    for (int i = 0; i < 2; ++i)
    {}
}

void readBeforeUnbracedLoop()
{
    int value = i;
/*CHECK:
                ^ undefined identifier
                ^ undefined identifier 'i'.
*/
    if (value != 0)
        for (int i = 0; i < 2; ++i)
        {}
}

void readAfterBracedLoop()
{
    {
        for (int i = 0; i < 2; ++i)
        {}
    }
    int value = i;
/*CHECK:
                ^ undefined identifier
                ^ undefined identifier 'i'.
*/
}

// Unbraced control-flow bodies introduce scopes too. A loop initializer inside a body must
// not become visible after the control-flow statement or inside a sibling `else` branch.

int readAfterIfBody(bool condition)
{
    if (condition)
        for (int i = 0; i < 2; ++i)
        {}
    return i;
/*CHECK:
           ^ undefined identifier
           ^ undefined identifier 'i'.
*/
}

int readInElseBranch(bool condition)
{
    if (condition)
        for (int i = 0; i < 2; ++i)
        {}
    else
        return i;
/*CHECK:
               ^ undefined identifier
               ^ undefined identifier 'i'.
*/
    return 0;
}

int readAfterWhileBody(bool condition)
{
    while (condition)
        for (int i = 0; i < 2; ++i)
        {
            condition = false;
        }
    return i;
/*CHECK:
           ^ undefined identifier
           ^ undefined identifier 'i'.
*/
}

int readAfterDoBody(bool condition)
{
    do
        for (int i = 0; i < 2; ++i)
        {}
    while (condition);
    return i;
/*CHECK:
           ^ undefined identifier
           ^ undefined identifier 'i'.
*/
}
