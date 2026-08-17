// class-inheritance.slang

// Test a class that conforms to an interface, instantiated with `new`.

// DISABLED: single-argument construction of a class with a user __init aborts with
// "could not resolve target declaration for call" (fires under slangi and every text
// target). Tracked by shader-slang/slang#12485. Re-enable once that is fixed.
//DISABLE_TEST:INTERPRET(filecheck=CHECK):

interface IDescribable
{
    int describe();
}

class Shape : IDescribable
{
    int sides;

    __init(int n)
    {
        sides = n;
    }

    int describe()
    {
        return sides;
    }
}

void main()
{
    Shape tri = new Shape(3);

    // CHECK: 3
    printf("%d\n", tri.describe());

    // CHECK: 3
    printf("%d\n", tri.sides);
}
