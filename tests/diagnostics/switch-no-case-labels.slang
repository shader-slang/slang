//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// A switch body with executable statements but no case/default label discards
// every one of them, so the whole body is unreachable and must warn (E41000).
// An empty body discards nothing and must stay silent.

RWStructuredBuffer<int> outputBuffer;

int noLabelsAtAll(int n)
{
    switch (n)
    {
        if (n == 1)
//CHECK:^^ E41000
            return 1;
        else if (n == 2)
            return 1337;
        else
            return -1;
    }
    return -1;
}

int multiStatementNoLabels(int n)
{
    switch (n)
    {
        outputBuffer[0] = 99;
//CHECK:^^^^^^^^^^^^ E41000
        outputBuffer[1] = 88;
        outputBuffer[2] = 77;
    }
    return 0;
}

int nestedBlockNoLabels(int n)
{
    switch (n)
    {
        {
            outputBuffer[3] = 66;
//CHECK:    ^^^^^^^^^^^^ E41000
        }
    }
    return 0;
}

// A variable declaration counts as a discarded statement, matching how the sibling
// unreachable-code sites classify one; a struct or typealias declaration never becomes a body
// statement, so it stays silent.
int declarationNoLabels(int n)
{
    switch (n)
    {
        int q = n;
//CHECK:^^^ E41000
    }
    return 0;
}

// A type declaration alone never becomes a body statement (the parser only wraps a `VarDeclBase`
// in a `DeclStmt`), so there is nothing to discard and nothing to report.
int typeDeclarationOnly(int n)
{
    switch (n)
    {
        struct S
        {
            int x;
        };
        typealias MyInt = int;
    }
    return 0;
}

// Declaring a variable of a locally declared type does produce a `DeclStmt`, so this is the case
// that exercises the predicate rather than the parser.
int structVariableDeclaration(int n)
{
    switch (n)
    {
        struct S
        {
            int x;
        } s;
//CHECK:  ^ E41000
    }
    return 0;
}

// A leading empty statement must not stop the search: the warning belongs to the statement after it.
int emptyStatementThenStatement(int n)
{
    switch (n)
    {
        ;
        outputBuffer[4] = 55;
//CHECK:^^^^^^^^^^^^ E41000
    }
    return 0;
}

// `break` is the whole body here. It still cannot be reached, so it is still reported.
int breakOnlyNoLabels(int n)
{
    switch (n)
    {
        break;
//CHECK:^^^^^ E41000
    }
    return 0;
}

// When the switch is itself unreachable, two warnings are expected: one for the dead `switch`
// statement from the surrounding block, one for its discarded body.
int deadSwitchNoLabels(int n)
{
    return 1;
    switch (n)
/*CHECK:
    ^^^^^^ E41000
*/
    {
        outputBuffer[5] = 44;
//CHECK:^^^^^^^^^^^^ E41000
    }
}

int emptyBody(int n)
{
    switch (n)
    {
    }
    return 0;
}

int emptyNestedBlock(int n)
{
    switch (n)
    {
        {
        }
    }
    return 0;
}

int emptyStatementOnly(int n)
{
    switch (n)
    {
        ;
    }
    return 0;
}

int normalSwitch(int n)
{
    switch (n)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    default:
        break;
    }
    return 3;
}

int defaultOnly(int n)
{
    switch (n)
    {
    default:
        break;
    }
    return 42;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int n = int(tid.x);
    outputBuffer[0] = noLabelsAtAll(n) + multiStatementNoLabels(n) + nestedBlockNoLabels(n) +
                      declarationNoLabels(n) + typeDeclarationOnly(n) +
                      structVariableDeclaration(n) + emptyStatementThenStatement(n) +
                      breakOnlyNoLabels(n) + deadSwitchNoLabels(n) + emptyBody(n) +
                      emptyNestedBlock(n) + emptyStatementOnly(n) + normalSwitch(n) +
                      defaultOnly(n);
}
