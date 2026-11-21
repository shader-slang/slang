//TEST:INTERPRET(filecheck=CHECK):
//DIAGNOSTIC_TEST:SIMPLE(diag=UNAVAILABLE,non-exhaustive):-target slang-vm -entry unavailableMain -stage dispatch -capability cpp
//DIAGNOSTIC_TEST:SIMPLE(diag=UNMATCHED,non-exhaustive):-target slang-vm -entry unmatchedMain -stage dispatch
// non-exhaustive: integer conversions warn before specialization and errors include notes.
// The VM has no implicit language target capability, so the unavailable case requests cpp.

enum Mode
{
    portable = 0,
    [require(hlsl)] hlslMode = 1,
}

int readMode<let tag : int>()
{
    return int(Mode(tag));
}

void main()
{
    // CHECK: portable: 0
    printf("portable: %d\n", readMode<0>());
}

void unavailableMain()
//UNAVAILABLE: enum conversion selects an unavailable case
{
    printf("%d\n", readMode<1>());
}

void unmatchedMain()
//UNMATCHED: enum conversion has no matching case after specialization
{
    printf("%d\n", readMode<42>());
}
