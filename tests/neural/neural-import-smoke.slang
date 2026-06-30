//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -experimental-feature

import slang.neural;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

// CHECK: OpEntryPoint
