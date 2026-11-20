// Regression test for https://github.com/shader-slang/slang/issues/14664.
//
// Bracket attributes on enum cases flow through the ordinary attribute
// checking path, so an unknown [require] capability name on a case is
// diagnosed with E36105 (unknown capability name) and an unrecognized
// attribute is diagnosed with warning E31000 (unknown attribute name) —
// neither is silently swallowed now that case attributes parse.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECKINVALID,non-exhaustive):-target hlsl -entry computeMain -stage compute

enum BadEnum
{
    [require(not_a_real_capability)]
    Bad = 0,
};

enum UnknownAttrEnum
{
    [Foo]
    Tagged = 0,
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    BadEnum v = BadEnum.Bad;
    UnknownAttrEnum w = UnknownAttrEnum.Tagged;
    if (v == BadEnum.Bad)
    {
    }
}

//CHECKINVALID: E36105
//CHECKINVALID: E31000
