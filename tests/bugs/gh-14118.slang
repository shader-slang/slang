// A local declared as the unbraced body of a statement is owned by the
// enclosing scope, but it must stay invisible to code that runs before its
// declaration, just like a local declared directly in a block (gh-14118).
// Each function reads `i` before an inner `i` is declared, and must read
// the parameter.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int readBeforeIfBody(int i, bool c)
{
    int value = 0;
    {
        value = i;
        if (c)
            int i = 1;
    }
    return value;
}

int readBeforeElseBody(int i, bool c)
{
    int value = 0;
    {
        value = i;
        if (!c)
        {
        }
        else
            int i = 1;
    }
    return value;
}

int readBeforeWhileBody(int i, bool c)
{
    int value = 0;
    {
        value = i;
        while (!c)
            int i = 1;
    }
    return value;
}

int readBeforeDoBody(int i, bool c)
{
    int value = 0;
    {
        value = i;
        do
            int i = 1;
        while (!c);
    }
    return value;
}

int readBeforeLabeledDecl(int i)
{
    int value = 0;
    {
        value = i;
    inner:
        int i = 1;
    }
    return value;
}

// The predicate and the side-effect expression of a `for` are checked before
// its body, so they must not see a local declared as the body.
int forPredicateBeforeBody(int i)
{
    int n = 0;
    for (int k = 0; k < i; k++, n++)
        int i = 100;
    return n;
}

int forSideEffectBeforeBody(int i)
{
    int value = 0;
    for (int k = 0; k < 1; k++, value = i)
        int i = 100;
    return value;
}

// The predicate of a `do` loop follows its body, so it sees a local declared
// as the body.
int doPredicateAfterBody()
{
    int n = 0;
    do
        int j = n++;
    while (j < 6);
    return n;
}

// Controls: a braced body and a direct declaration in the block.
int readBeforeBracedBody(int i, bool c)
{
    int value = 0;
    {
        value = i;
        if (c)
        {
            int i = 1;
        }
    }
    return value;
}

int readBeforeDirectDecl(int i)
{
    int value = 0;
    {
        value = i;
        int i = 1;
    }
    return value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 7
    outputBuffer[0] = readBeforeIfBody(7, true);
    // CHECK-NEXT: 7
    outputBuffer[1] = readBeforeElseBody(7, true);
    // CHECK-NEXT: 7
    outputBuffer[2] = readBeforeWhileBody(7, true);
    // CHECK-NEXT: 7
    outputBuffer[3] = readBeforeDoBody(7, true);
    // CHECK-NEXT: 7
    outputBuffer[4] = readBeforeLabeledDecl(7);
    // CHECK-NEXT: 7
    outputBuffer[5] = forPredicateBeforeBody(7);
    // CHECK-NEXT: 7
    outputBuffer[6] = forSideEffectBeforeBody(7);
    // CHECK-NEXT: 7
    outputBuffer[7] = doPredicateAfterBody();
    // CHECK-NEXT: 7
    outputBuffer[8] = readBeforeBracedBody(7, true);
    // CHECK-NEXT: 7
    outputBuffer[9] = readBeforeDirectDecl(7);
}
