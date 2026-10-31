// A local declared as the unbraced body of a statement is owned by the
// enclosing scope, but it must stay invisible to code that comes before its
// declaration in the source, just like a local declared directly in a block
// (gh-14118).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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

Optional<int> makeOptional(int v)
{
    return v;
}

// The `else` branch of an `if (let ...)` is parsed in the scope that holds the
// optional temporary, so its unbraced local must stay hidden from the branch
// and the initializer that come before it.
int readBeforeIfLetElseBody(int i)
{
    int value = 0;
    if (let x = makeOptional(1))
        value = i;
    else
        int i = 1;
    return value;
}

int ifLetInitializerBeforeElseBody(int i)
{
    int value = 0;
    if (let x = makeOptional(i))
        value = x;
    else
        int i = 1;
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

int readBeforeDeferBody(int i)
{
    int value = 0;
    {
        value = i;
        defer int i = 1;
    }
    return value;
}

// The predicate and the side-effect expression of a `for` precede its body in
// the source, so they must not see a local declared as the body.
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

// A `while` predicate precedes its body, so it must not see a local declared
// as the body.
int whilePredicateBeforeBody(int i)
{
    int n = 0;
    while (n < i)
        int i = n++;
    return n;
}

// An unbraced body has no scope of its own today, so its local belongs to the
// enclosing scope, and the predicate of a `do` loop, which follows the body,
// sees it. Giving unbraced bodies their own scope (option B on gh-14118) would
// change this case.
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
    outputBuffer[4] = readBeforeIfLetElseBody(7);
    // CHECK-NEXT: 7
    outputBuffer[5] = ifLetInitializerBeforeElseBody(7);
    // CHECK-NEXT: 7
    outputBuffer[6] = readBeforeLabeledDecl(7);
    // CHECK-NEXT: 7
    outputBuffer[7] = readBeforeDeferBody(7);
    // CHECK-NEXT: 7
    outputBuffer[8] = forPredicateBeforeBody(7);
    // CHECK-NEXT: 7
    outputBuffer[9] = forSideEffectBeforeBody(7);
    // CHECK-NEXT: 7
    outputBuffer[10] = whilePredicateBeforeBody(7);
    // CHECK-NEXT: 7
    outputBuffer[11] = doPredicateAfterBody();
    // CHECK-NEXT: 7
    outputBuffer[12] = readBeforeBracedBody(7, true);
    // CHECK-NEXT: 7
    outputBuffer[13] = readBeforeDirectDecl(7);
}
