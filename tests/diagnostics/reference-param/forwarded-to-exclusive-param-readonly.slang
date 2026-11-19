// A `__ref_readonly` parameter names storage the callee cannot write, so forwarding it to an
// `inout` parameter is rejected by the existing l-value rule; the exclusivity warning must
// not pile onto that error. This case lives in its own file because the always-on l-value
// error would break the default-off and suppression runs of the companion test
// (`forwarded-to-exclusive-param.slang`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=-1): -target hlsl -entry main -stage compute -Wall -experimental-feature

groupshared int g[4];

void innerInout(inout int x) { x += 1; }

void viaReadonly(__ref_readonly int r)
{
    innerInout(r);
//CHECK:       ^ argument must be l-value
//CHECK:       ^ argument passed to parameter '0' must be l-value
}

[numthreads(4, 1, 1)]
void main(uint i : SV_GroupIndex)
{
    viaReadonly(g[i]);
}
