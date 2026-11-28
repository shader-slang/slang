//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG,exit-code=0): -target wgsl -entry computeMain -stage compute

// A `precise` loop counter keeps the `loop` form, so its declaration still reports that WGSL
// drops `precise`. The `for` header has no place for the qualifier or the warning.

RWStructuredBuffer<uint> buffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint s = 0;
    for (precise int i = 0; i < 4; i++)
        s += buffer[i];
    buffer[4] = s;
}

// WGSL-NOT: for (var
// WGSL: loop

//DIAG: warning E56005
