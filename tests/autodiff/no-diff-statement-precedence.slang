//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#pragma warning(disable: 41024) // Comma expressions are intentional in this parser test.

RWStructuredBuffer<Atomic<float>> values;
float nonDiff(float x) { return x * x; }

[Differentiable]
float prefixesBindToTheCall(float x)
{
    no_diff float constant = 1.0f;
    no_diff nonDiff(x) + 1.0f;
    no_diff(nonDiff(x)) + 1.0f;
    (no_diff nonDiff(x)) + 1.0f;
    no_diff values[0].add(x) + 1.0f;
    no_diff(values[0].add(x)) + 1.0f;
    no_diff values[0].add(x), values[1].add(detach(x));
    no_diff values[0].load() + 1.0f;
    return x * x + constant;
}

[Differentiable]
float onlyThePrefixedCallIsAcknowledged(float x)
{
    no_diff values[0].add(x), values[1].add(x);
//CHECK:                                   ^ derivative is lost in atomic operation
//CHECK:                                   ^ derivative is lost when an atomic operation writes it to non-differentiable memory, use 'no_diff' or 'detach()' to clarify intention.
    return x * x;
}
