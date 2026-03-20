//TEST:INTERPRET(filecheck=CHECK):

// Verify a forwarded `nonempty(...)` witness survives generic specialization
// and still enables non-empty-only pack queries in the callee.

int consumeFirst<let each D : int>() where nonempty(D)
{
    return __first(D);
}

int forwardFirst<let each D : int>() where nonempty(D)
{
    return consumeFirst<D>();
}

int forwardTwice<let each D : int>() where nonempty(D)
{
    return forwardFirst<D>();
}

void main()
{
    printf("%d\n", forwardFirst<3, 5, 7>());
    // CHECK: 3

    printf("%d\n", forwardTwice<11, 13>());
    // CHECK: 11
}
