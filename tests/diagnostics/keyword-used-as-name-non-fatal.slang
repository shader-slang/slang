//TEST:INTERPRET(filecheck=CHECK):

// Using a type keyword as a name is only a *warning*, not an error: the
// declaration still type-checks, lowers, and runs. (A name like `struct` can't
// be referenced afterwards, but the rest of the program must compile and
// execute.) This pins the "warn, don't error" contract that the parser-side
// diagnostic relies on.

void main()
{
    int structVal = 41;
    int struct = 42; // warns (E20103) but must still compile and run
    printf("%d\n", structVal + 1);
//CHECK: 42
}
