//TEST:SIMPLE_LINE(filecheck=WARNCHECK):-entry warnMain -stage compute -target spirv-asm

[shader("compute")]
[numthreads(1,1,1)]
void warnMain()
{
    float4 v = (0.f, 0.f, 0.f, 1.f);
    // WARNCHECK: [[#@LINE-1]]
}
