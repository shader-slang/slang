//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

struct PartlyDiffable : IDifferentiable
{
    no_diff float noDiffMember;
    float diffMember;
}

float func(PartlyDiffable.Differential x)
{
   // CHECK: ([[# @LINE+1]]): error 30027: 'noDiffMember' is not a member of 'PartlyDiffable.Differential'.
    return x.noDiffMember;
}

float func1(PartlyDiffable.Differential x)
{
   // CHECK-NOT: ([[# @LINE+1]]): error 30027: 'diffMember.
    return x.diffMember;
}

RWStructuredBuffer<float> out;

[shader("compute")]
void compute()
{
    // CHECK:  ([[# @LINE+1]]): error 39999: no overload for 'Differential' applicable to arguments of type (float, float)
    var diff = PartlyDiffable.Differential(1.0f, 2.0f);
    out[0] = func(diff);
    out[1] = func1(diff);
}
