//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 count : SV_WorkgroupCount)
{
}

// CHECK: system value semantic '{{[sS][vV]_[wW]orkgroup[cC]ount}}' is not supported for the current target.
