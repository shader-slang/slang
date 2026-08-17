// class-reference-semantics.slang

// Test that class assignment shares the reference: modifying via one
// reference affects the other (unlike structs which are value types).
// NOTE: Uses single-field class due to interpreter crash with multi-field classes.

// DISABLED: single-argument construction of a class with a user __init aborts with
// "could not resolve target declaration for call" (fires under slangi and every text
// target). Tracked by shader-slang/slang#12485. Re-enable once that is fixed.
//DISABLE_TEST:INTERPRET(filecheck=CHECK):

class Box
{
    int contents;

    __init(int val)
    {
        contents = val;
    }
}

void modifyBox(Box b, int newVal)
{
    b.contents = newVal;
}

void main()
{
    Box a = new Box(10);

    // CHECK: 10
    printf("%d\n", a.contents);

    // Pass to function — reference semantics means function modifies original
    modifyBox(a, 99);

    // CHECK: 99
    printf("%d\n", a.contents);
}
