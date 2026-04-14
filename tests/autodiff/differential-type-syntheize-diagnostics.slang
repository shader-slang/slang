//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -line-directive-mode none

struct PartlyDiffable : IDifferentiable
{
    no_diff float noDiffMember;
    float diffMember;
}

float func(PartlyDiffable.Differential x)
{
    return x.noDiffMember;
//CHECK:     ^^^^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^^^^ 'noDiffMember' is not a member of 'PartlyDiffable.Differential'.
}

float func1(PartlyDiffable.Differential x)
{
    return x.diffMember;
}

RWStructuredBuffer<float> out;

[shader("compute")]
void compute()
{
    var diff = PartlyDiffable.Differential(1.0f, 2.0f);
//CHECK:                                  ^ no overload for 'Differential' applicable to arguments of type (float, float)
    out[0] = func(diff);
    out[1] = func1(diff);
}
