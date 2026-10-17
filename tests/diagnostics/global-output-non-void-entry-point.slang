// Every entry point that reaches a module-scope `out` varying must return `void`, whichever
// order the entry points are declared or listed in (shader-slang/slang#13687).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -whole-program
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -whole-program -entry fsB -stage fragment -entry fsA -stage fragment

out float4 gColor : SV_Target;

void writeColor()
{
    gColor = float4(1, 0, 0, 1);
}

[shader("fragment")]
void fsA()
{
    writeColor();
}

[shader("fragment")] float4 fsB() : SV_Target
//CHECK:                    ^^^ error E58001
//CHECK:                    ^^^ entry point must return 'void' when global output variables are present.
{
    writeColor();
    return 0;
}
