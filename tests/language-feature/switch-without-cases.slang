//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A `switch` whose body holds executable statements but no `case`/`default`
// label is a silent miscompile: nothing in the body is reachable without a
// label, so lowering discards all of it. The frontend must now diagnose it.

// Error: the body is an `if`/`else` chain with no `case`/`default` label, so
// every statement here would be silently dropped.
int discardsBodyWithoutLabels(int n)
{
    switch (n)
    {
        if (n == 1) return 1;
        else if (n == 2) return 1337;
        else if (n < 0) return 69420;
        else return -1;
    }
    //CHECK: no 'case' or 'default' labels
    return -1;
}

// Error: a single ordinary statement with no label is dropped just the same.
int discardsSingleStatement(int n)
{
    switch (n)
    {
        return 1;
    }
    //CHECK: no 'case' or 'default' labels
    return -1;
}

// Legal (must stay silent): a normal `switch` with `case`/`default` labels.
int normalSwitch(int n)
{
    switch (n)
    {
    case 1:
        return 1;
    case 2:
        return 1337;
    default:
        return -1;
    }
}

// Legal (must stay silent): an empty `switch` body has nothing to discard.
void emptySwitch(int n)
{
    switch (n)
    {
    }
}
