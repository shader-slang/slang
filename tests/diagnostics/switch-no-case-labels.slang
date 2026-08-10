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
                      declarationNoLabels(n) + typeDeclarationOnly(n) + emptyBody(n) +
                      emptyNestedBlock(n) + emptyStatementOnly(n) + normalSwitch(n) +
                      defaultOnly(n);
}
