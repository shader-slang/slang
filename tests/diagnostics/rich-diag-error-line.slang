// The first diagnostic is E30015 "undefined identifier" at the statement below; the
// SIMPLE_LINE lane pins its parsed line, the SIMPLE lane pins the raw rich header.
//TEST:SIMPLE_LINE(filecheck=ERRCHECK):-entry main -stage compute -target spirv-asm
//TEST:SIMPLE(filecheck=ERRHDR,exit-code=-1):-entry main -stage compute -target spirv-asm

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    undefined_thing;
    // ERRCHECK: [[#@LINE-1]]
}
// ERRHDR: error[E30015]: undefined identifier
