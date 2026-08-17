// class-basic-definition.slang

// Test basic class definition, instantiation with `new`, and field access.

// DISABLED: single-argument construction of a class with a user __init aborts with
// "could not resolve target declaration for call" (fires under slangi and every text
// target). Tracked by shader-slang/slang#12485. Re-enable once that is fixed.
//DISABLE_TEST:INTERPRET(filecheck=CHECK):

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
