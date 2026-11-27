// The first diagnostics are E41024 "comma operator used in expression" warnings at the
// initializer below; the SIMPLE_LINE lane pins the first parsed line, the SIMPLE lane pins
// the raw rich header. The WARNPOS lane prepends the positionless E00018 option warning,
// so the first parsed record has no location (flushed on the next header) — line 0.
//TEST:SIMPLE_LINE(filecheck=WARNCHECK):-entry warnMain -stage compute -target spirv-asm
//TEST:SIMPLE_LINE(filecheck=WARNPOS):-entry warnMain -stage compute -target spirv-asm -separate-debug-info
//TEST:SIMPLE(filecheck=WARNHDR):-entry warnMain -stage compute -target spirv-asm

[shader("compute")]
[numthreads(1,1,1)]
void warnMain()
{
    float4 v = (0.f, 0.f, 0.f, 1.f);
    // WARNCHECK: [[#@LINE-1]]
}
// WARNPOS: 0
// WARNHDR: warning[E41024]: comma operator used in expression
