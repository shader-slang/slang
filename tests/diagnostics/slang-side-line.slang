//TEST:SIMPLE_LINE(filecheck=ERRCHECK):-entry main -stage compute -target spirv-asm

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    undefined_thing;
    // ERRCHECK: [[#@LINE-1]]
}
