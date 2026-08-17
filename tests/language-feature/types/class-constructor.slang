// class-constructor.slang

// Test class constructors (__init) with default and parameterized forms.

// DISABLED: single-argument construction of a class with a user __init aborts with
// "could not resolve target declaration for call" (fires under slangi and every text
// target). Tracked by shader-slang/slang#12485. Re-enable once that is fixed.
//DISABLE_TEST:INTERPRET(filecheck=CHECK):

class Counter
{
    int value;

    __init()
    {
        value = 0;
    }

    __init(int startValue)
    {
        value = startValue;
    }
}

void main()
{
    // Default constructor
    Counter c0 = new Counter();

    // CHECK: 0
    printf("%d\n", c0.value);

    // Parameterized constructor
    Counter c1 = new Counter(100);

    // CHECK: 100
    printf("%d\n", c1.value);
}
