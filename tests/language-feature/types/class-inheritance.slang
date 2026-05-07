// class-inheritance.slang

// Test class inheriting from a base class and implementing an interface.
// NOTE: Uses single-field per class due to interpreter crash with multi-field classes.

//TEST:INTERPRET(filecheck=CHECK):

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
