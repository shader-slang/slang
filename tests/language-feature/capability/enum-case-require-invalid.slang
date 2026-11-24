//DIAGNOSTIC_TEST:SIMPLE(diag=FUNCTION,non-exhaustive):-target hlsl -DTEST_FUNCTION=1
//DIAGNOSTIC_TEST:SIMPLE(diag=CONTRACT,non-exhaustive):-target hlsl -DTEST_FUNCTION=0
// non-exhaustive: capability errors also include declaration/use notes.

#if TEST_FUNCTION
enum Mode
{
    [require(hlsl)] A = 0,
}

// An explicit requirement must cover references in the declaration's body.
[require(spirv)]
int incompatible()
//FUNCTION: dependencies not compatible on target
{
    return int(Mode.A);
}

#else
// Compatible target requirements still need explicit coverage of the referenced feature.
public enum Feature
{
    [require(sm_6_6)] A = 0,
    [require(sm_6_0)] B = A,
//CONTRACT: undeclared capability used
}

public struct Holder<let V : Feature>
{
    int payload;
}

[require(sm_6_0)]
public typealias TooWeak = Holder<Feature.A>;
//CONTRACT: undeclared capability used
#endif
