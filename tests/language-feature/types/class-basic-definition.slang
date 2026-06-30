// class-basic-definition.slang

// Test basic class definition, instantiation with `new`, and field access.

//DISABLE_TEST:INTERPRET(filecheck=CHECK):
// DISABLED: interpreter crash "could not resolve target declaration for call" on class constructors

class Animal
{
    int legs;

    __init(int l)
    {
        legs = l;
    }

    int score()
    {
        return legs * 10;
    }
}

void main()
{
    Animal cat = new Animal(4);

    // CHECK: 4
    printf("%d\n", cat.legs);

    // CHECK: 40
    printf("%d\n", cat.score());
}
