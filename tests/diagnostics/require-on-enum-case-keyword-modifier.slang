// A keyword modifier before an enum case is a parse error. Bracket attributes
// are the only modifier syntax a case accepts, so `static` here is parsed as
// the case's own name and `FirstValue` is unexpected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKKEYWORDMOD,non-exhaustive):-target hlsl -entry computeMain -stage compute

enum E
{
    static FirstValue = 0,
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
}

//CHECKKEYWORDMOD: E20001
