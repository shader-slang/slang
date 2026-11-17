//TEST:SIMPLE_LINE(filecheck=NOTECHECK):-entry main -stage compute -target spirv-asm

[shader("compute")]
[numthreads(1,1,1)]
void main() { }

[shader("compute")]
[numthreads(1,1,1)]
void main() { }
// NOTECHECK: [[#@LINE-1]]
